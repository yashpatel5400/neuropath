/*****************************************************************
 * File: perceptron.hh
 * Created on: 13-May-2017
 * Author: Yash Patel
 * Description: Perceptron branch predictor for the gem5 simulator
 * environment.
 ****************************************************************/

#ifndef __CPU_PRED_NEUROBRANCH_PRED_HH__
#define __CPU_PRED_NEUROBRANCH_PRED_HH__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

/* an entry in a BTB */
struct bpred_btb_ent_t {
  md_addr_t addr;		/* address of branch being tracked */
  enum md_opcode op;		/* opcode of branch corresp. to addr */
  md_addr_t target;		/* last destination of branch when taken */
  struct bpred_btb_ent_t *prev, *next; /* lru chaining pointers */
};

/***************************************************************
Created the new structure for perceptron
****************************************************************/
/* direction predictor def */
struct bpred_dir_t {
  int weight_index;			/* number  of weight indexes (K) */
  int weight_bits;    			/* number of bits per weight  = 8 bits*/
  int bhr_length; 			/* history length for the global history shift register (N)*/
  int output;	                   	/* to store the output of each lookup and use it back in update */
  signed int weights_table[500][500];	/* every entry is an element of array of weights */	
  signed int masks_table[100];		/* array of global bhr to be used for dot product with weights in lookup*/
  int index;				/* the index value = (baddr>>2) % (size weight_index)*/
} perceptron;
};

/* branch predictor def */
struct bpred_t {
  struct bpred_dir_t *bimod;	  /* first direction predictor */
  struct {
    int sets;			/* num BTB sets */
    int assoc;			/* BTB associativity */
    struct bpred_btb_ent_t *btb_data; /* BTB addr-prediction table */
  } btb;

  struct {
    int size;			/* return-address stack size */
    int tos;			/* top-of-stack */
    struct bpred_btb_ent_t *stack; /* return-address stack */
  } retstack;

  /* stats */
  counter_t addr_hits;		/* num correct addr-predictions */
  counter_t dir_hits;		/* num correct dir-predictions (incl addr) */
  counter_t used_ras;		/* num RAS predictions used */
  counter_t used_bimod;		/* num bimodal predictions used (BPredComb) */
  counter_t used_2lev;		/* num 2-level predictions used (BPredComb) */
  counter_t jr_hits;		/* num correct addr-predictions for JR's */
  counter_t jr_seen;		/* num JR's seen */
  counter_t jr_non_ras_hits;	/* num correct addr-preds for non-RAS JR's */
  counter_t jr_non_ras_seen;	/* num non-RAS JR's seen */
  counter_t misses;		/* num incorrect predictions */

  counter_t lookups;		/* num lookups */
  counter_t retstack_pops;	/* number of times a value was popped */
  counter_t retstack_pushes;	/* number of times a value was pushed */
  counter_t ras_hits;		/* num correct return-address predictions */
};

/* branch predictor update information */
struct bpred_update_t {
  char *pdir1;		/* direction-1 predictor counter */
  char *pdir2;		/* direction-2 predictor counter */
  char *pmeta;		/* meta predictor counter */
  struct {		/* predicted directions */
    unsigned int ras    : 1;	/* RAS used */
    unsigned int bimod  : 1;    /* bimodal predictor */
    unsigned int twolev : 1;    /* 2-level predictor */
    unsigned int meta   : 1;    /* meta predictor (0..bimod / 1..2lev) */
  } dir;
};

/* create a branch predictor */
struct bpred_t *			/* branch predictory instance */
bpred_create(
			 unsigned int bimod_size,	/* bimod table size */
			 unsigned int l1size,	/* level-1 table size */
			 unsigned int l2size,	/* level-2 table size */
			 unsigned int shift_width,	/* history register width */
			 unsigned int btb_sets,	/* number of sets in BTB */ 
			 unsigned int btb_assoc,	/* BTB associativity */
			 unsigned int retstack_size);/* num entries in ret-addr stack */

/* create a branch direction predictor */
struct bpred_dir_t *		/* branch direction predictor instance */
bpred_dir_create (
  unsigned int l1size,		/* level-1 table size */
  unsigned int l2size,		/* level-2 table size (if relevant) */
  unsigned int shift_width);/* history register width */

/* print branch predictor configuration */
void
bpred_config(struct bpred_t *pred,	/* branch predictor instance */
	     FILE *stream);		/* output stream */

/* print predictor stats */
void
bpred_stats(struct bpred_t *pred,	/* branch predictor instance */
	    FILE *stream);		/* output stream */

/* register branch predictor stats */
void
bpred_reg_stats(struct bpred_t *pred,	/* branch predictor instance */
		struct stat_sdb_t *sdb);/* stats database */

/* reset stats after priming, if appropriate */
void bpred_after_priming(struct bpred_t *bpred);

/* probe a predictor for a next fetch address, the predictor is probed
   with branch address BADDR, the branch target is BTARGET (used for
   static predictors), and OP is the instruction opcode (used to simulate
   predecode bits; a pointer to the predictor state entry (or null for jumps)
   is returned in *DIR_UPDATE_PTR (used for updating predictor state),
   and the non-speculative top-of-stack is returned in stack_recover_idx 
   (used for recovering ret-addr stack after mis-predict).  */
md_addr_t				/* predicted branch target addr */
bpred_lookup(struct bpred_t *pred,	/* branch predictor instance */
	     md_addr_t baddr,		/* branch address */
	     md_addr_t btarget,		/* branch target if taken */
	     enum md_opcode op,		/* opcode of instruction */
	     int is_call,		/* non-zero if inst is fn call */
	     int is_return,		/* non-zero if inst is fn return */
	     struct bpred_update_t *dir_update_ptr, /* pred state pointer */
	     int *stack_recover_idx);	/* Non-speculative top-of-stack;
					 * used on mispredict recovery */

/* Speculative execution can corrupt the ret-addr stack.  So for each
 * lookup we return the top-of-stack (TOS) at that point; a mispredicted
 * branch, as part of its recovery, restores the TOS using this value --
 * hopefully this uncorrupts the stack. */
void
bpred_recover(struct bpred_t *pred,	/* branch predictor instance */
	      int stack_recover_idx);	/* Non-speculative top-of-stack;
					                 * used on mispredict recovery */

/* update the branch predictor, only useful for stateful predictors; updates
   entry for instruction type OP at address BADDR.  BTB only gets updated
   for branches which are taken.  Inst was determined to jump to
   address BTARGET and was taken if TAKEN is non-zero.  Predictor 
   statistics are updated with result of prediction, indicated by CORRECT and 
   PRED_TAKEN, predictor state to be updated is indicated by *DIR_UPDATE_PTR 
   (may be NULL for jumps, which shouldn't modify state bits).  Note if
   bpred_update is done speculatively, branch-prediction may get polluted. */
void
bpred_update(struct bpred_t *pred,	/* branch predictor instance */
	     md_addr_t baddr,		/* branch address */
	     md_addr_t btarget,		/* resolved branch target */
	     int taken,			/* non-zero if branch was taken */
	     int pred_taken,		/* non-zero if branch was pred taken */
	     int correct,		/* was earlier prediction correct? */
	     enum md_opcode op,		/* opcode of instruction */
	     struct bpred_update_t *dir_update_ptr); /* pred state pointer */

#endif 
