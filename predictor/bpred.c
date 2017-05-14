/*****************************************************************
 * File: perceptron.cc
 * Created on: 13-May-2017
 * Author: Yash Patel
 * Description: Perceptron branch predictor for the gem5 simulator
 * environment.
 ****************************************************************/

#include "bpred.h"  

/* create a branch predictor */
struct bpred_t *			/* branch predictory instance */
bpred_create(unsigned int bimod_size,	/* bimod table size */
			 unsigned int l1size,	/* 2lev l1 table size */
			 unsigned int l2size,	/* 2lev l2 table size */
			 unsigned int shift_width,	/* history register width */
			 unsigned int btb_sets,	/* number of sets in BTB */ 
			 unsigned int btb_assoc,	/* BTB associativity */
			 unsigned int retstack_size) /* num entries in ret-addr stack */
{
  struct bpred_t *pred;

  if (!(pred = calloc(1, sizeof(struct bpred_t))))
    fatal("out of virtual memory");
    
  /***************************************************************
  created a new case named BPredPerceptron getting values from 
  bpred_dir_create assigned in sim-outorder.c
  l1size = K
  l2size = no. of bits for each weight entry = 8 bits
  shift_width = size of global BHR
  ****************************************************************/
  pred->bimod = bpred_dir_create(l1size, l2size, shift_width, 0);   
      
  /***************************************************************
    allocation of BTB and RAS for perceptron
  ****************************************************************/
  int i;

  /* allocate BTB */
  if (!btb_sets || (btb_sets & (btb_sets-1)) != 0)
	fatal("number of BTB sets must be non-zero and a power of two");
  if (!btb_assoc || (btb_assoc & (btb_assoc-1)) != 0)
	fatal("BTB associativity must be non-zero and a power of two");

  if (!(pred->btb.btb_data = calloc(btb_sets * btb_assoc,
									sizeof(struct bpred_btb_ent_t))))
	fatal("cannot allocate BTB");

  pred->btb.sets = btb_sets;
  pred->btb.assoc = btb_assoc;

  if (pred->btb.assoc > 1)
	for (i=0; i < (pred->btb.assoc*pred->btb.sets); i++)
	  {
		if (i % pred->btb.assoc != pred->btb.assoc - 1)
		  pred->btb.btb_data[i].next = &pred->btb.btb_data[i+1];
		else
		  pred->btb.btb_data[i].next = NULL;
	    
		if (i % pred->btb.assoc != pred->btb.assoc - 1)
		  pred->btb.btb_data[i+1].prev = &pred->btb.btb_data[i];
	  }

  /* allocate retstack */
  if ((retstack_size & (retstack_size-1)) != 0)
	fatal("Return-address-stack size must be zero or a power of two");
      
  pred->retstack.size = retstack_size;
  if (retstack_size)
	if (!(pred->retstack.stack = calloc(retstack_size, 
										sizeof(struct bpred_btb_ent_t))))
	  fatal("cannot allocate return-address-stack");
  pred->retstack.tos = retstack_size - 1;

  return pred;
}

/* create a branch direction predictor */
struct bpred_dir_t *		/* branch direction predictor instance */
bpred_dir_create (unsigned int l1size,	 	/* level-1 table size */
				  unsigned int l2size,	 	/* level-2 table size (if relevant) */
				  unsigned int shift_width)	/* history register width */
{
  struct bpred_dir_t *pred_dir;
  unsigned int cnt;
  int flipflop;

  if (!(pred_dir = calloc(1, sizeof(struct bpred_dir_t))))
    fatal("out of virtual memory");

  cnt = -1;
  
  /***************************************************************
  Checking for false parameter values for perceptron
  ****************************************************************/
  if (!l1size)
	fatal("number of perceptrons, `%d', must be non-zero and positive",
		  l1size);
  if (!l2size)
	fatal("number of perceptrons, `%d', must be non-zero and positive",
		  l2size);
  if (!shift_width)
	fatal("shift register width, `%d', must be non-zero and positive",
		  shift_width);
	
  pred_dir->perceptron.weight_index = l1size;
  pred_dir->perceptron.weight_bits = l2size;
  pred_dir->perceptron.bhr_length = shift_width;
   
  /***************************************************************
  Initializing weight table to all 0 and global BHR (mask_table) to all 1
  ****************************************************************/
  int i,j;

  for (i = 0; i < pred_dir->perceptron.index; i++) {
	for (j=0; j < shift_width; j++)
	  pred_dir->perceptron.weights_table[i][j] = 0;	 	
  }
        
  for (cnt = 0; cnt < shift_width; cnt++) {
	pred_dir->perceptron.masks_table[cnt] = 1;
  }

  return pred_dir;
}

/* register branch predictor stats */
void
bpred_reg_stats(struct bpred_t *pred,	/* branch predictor instance */
				struct stat_sdb_t *sdb)	/* stats database */
{
  char buf[512], buf1[512], *name;

  /* get a name for this predictor */
  /***************************************************************
  Printing of stats
  ****************************************************************/

  name = "bpred_perceptron";

  sprintf(buf, "%s.lookups", name);
  stat_reg_counter(sdb, buf, "total number of bpred lookups",
				   &pred->lookups, 0, NULL);
  sprintf(buf, "%s.updates", name);
  sprintf(buf1, "%s.dir_hits + %s.misses", name, name);
  stat_reg_formula(sdb, buf, "total number of updates", buf1, "%12.0f");
  sprintf(buf, "%s.addr_hits", name);
  stat_reg_counter(sdb, buf, "total number of address-predicted hits", 
				   &pred->addr_hits, 0, NULL);
  sprintf(buf, "%s.dir_hits", name);
  stat_reg_counter(sdb, buf, 
				   "total number of direction-predicted hits "
				   "(includes addr-hits)", 
				   &pred->dir_hits, 0, NULL);
  if (pred->class == BPredComb)
    {
      sprintf(buf, "%s.used_bimod", name);
      stat_reg_counter(sdb, buf, 
					   "total number of bimodal predictions used", 
					   &pred->used_bimod, 0, NULL);
      sprintf(buf, "%s.used_2lev", name);
      stat_reg_counter(sdb, buf, 
					   "total number of 2-level predictions used", 
					   &pred->used_2lev, 0, NULL);
    }
  sprintf(buf, "%s.misses", name);
  stat_reg_counter(sdb, buf, "total number of misses", &pred->misses, 0, NULL);
  sprintf(buf, "%s.jr_hits", name);
  stat_reg_counter(sdb, buf,
				   "total number of address-predicted hits for JR's",
				   &pred->jr_hits, 0, NULL);
  sprintf(buf, "%s.jr_seen", name);
  stat_reg_counter(sdb, buf,
				   "total number of JR's seen",
				   &pred->jr_seen, 0, NULL);
  sprintf(buf, "%s.jr_non_ras_hits.PP", name);
  stat_reg_counter(sdb, buf,
				   "total number of address-predicted hits for non-RAS JR's",
				   &pred->jr_non_ras_hits, 0, NULL);
  sprintf(buf, "%s.jr_non_ras_seen.PP", name);
  stat_reg_counter(sdb, buf,
				   "total number of non-RAS JR's seen",
				   &pred->jr_non_ras_seen, 0, NULL);
  sprintf(buf, "%s.bpred_addr_rate", name);
  sprintf(buf1, "%s.addr_hits / %s.updates", name, name);
  stat_reg_formula(sdb, buf,
				   "branch address-prediction rate (i.e., addr-hits/updates)",
				   buf1, "%9.4f");
  sprintf(buf, "%s.bpred_dir_rate", name);
  sprintf(buf1, "%s.dir_hits / %s.updates", name, name);
  stat_reg_formula(sdb, buf,
				   "branch direction-prediction rate (i.e., all-hits/updates)",
				   buf1, "%9.4f");
  sprintf(buf, "%s.bpred_jr_rate", name);
  sprintf(buf1, "%s.jr_hits / %s.jr_seen", name, name);
  stat_reg_formula(sdb, buf,
				   "JR address-prediction rate (i.e., JR addr-hits/JRs seen)",
				   buf1, "%9.4f");
  sprintf(buf, "%s.bpred_jr_non_ras_rate.PP", name);
  sprintf(buf1, "%s.jr_non_ras_hits.PP / %s.jr_non_ras_seen.PP", name, name);
  stat_reg_formula(sdb, buf,
				   "non-RAS JR addr-pred rate (ie, non-RAS JR hits/JRs seen)",
				   buf1, "%9.4f");
  sprintf(buf, "%s.retstack_pushes", name);
  stat_reg_counter(sdb, buf,
				   "total number of address pushed onto ret-addr stack",
				   &pred->retstack_pushes, 0, NULL);
  sprintf(buf, "%s.retstack_pops", name);
  stat_reg_counter(sdb, buf,
				   "total number of address popped off of ret-addr stack",
				   &pred->retstack_pops, 0, NULL);
  sprintf(buf, "%s.used_ras.PP", name);
  stat_reg_counter(sdb, buf,
				   "total number of RAS predictions used",
				   &pred->used_ras, 0, NULL);
  sprintf(buf, "%s.ras_hits.PP", name);
  stat_reg_counter(sdb, buf,
				   "total number of RAS hits",
				   &pred->ras_hits, 0, NULL);
  sprintf(buf, "%s.ras_rate.PP", name);
  sprintf(buf1, "%s.ras_hits.PP / %s.used_ras.PP", name, name);
  stat_reg_formula(sdb, buf,
				   "RAS prediction rate (i.e., RAS hits/used RAS)",
				   buf1, "%9.4f");
}

void
bpred_after_priming(struct bpred_t *bpred)
{
  if (bpred == NULL)
    return;

  bpred->lookups = 0;
  bpred->addr_hits = 0;
  bpred->dir_hits = 0;
  bpred->used_ras = 0;
  bpred->used_bimod = 0;
  bpred->used_2lev = 0;
  bpred->jr_hits = 0;
  bpred->jr_seen = 0;
  bpred->misses = 0;
  bpred->retstack_pops = 0;
  bpred->retstack_pushes = 0;
  bpred->ras_hits = 0;
}

/* predicts a branch direction */
char *						                    /* pointer to counter */
bpred_dir_lookup(struct bpred_dir_t *pred_dir,	/* branch dir predictor inst */
				 md_addr_t baddr)		        /* branch address */
{
  unsigned char *p = NULL;

  /***************************************************************
  perceptron lookup algorithm
  ***************************************************************/

  int	index, i;
  signed int product[100], sum = 0;
  signed int output = 0;
  int *entry;
			
  product[0]=0;
  index = (baddr >> MD_BR_SHIFT) % pred_dir->config.perceptron.weight_index; 
  pred_dir->config.perceptron.index = index;
			
  pred_dir->config.perceptron.masks_table[0] =1; //set 0th bit of BHR = 1 always to provide a bias (given in paper)
  /***************************************************************
  Calculating yout = w[0] + sum (w[i]*x[i])
  ****************************************************************/	
  for (i=0; i < pred_dir->config.perceptron.bhr_length; i++)
	{
	  product[i] = (pred_dir->config.perceptron.weights_table[index][i]) * (pred_dir->config.perceptron.masks_table[i]);
	  output += product[i];
	}
			
  /* get pointers to that perceptron and its weights */			
  pred_dir->config.perceptron.output = output;
  p = &pred_dir->config.perceptron.weights_table[index][i];
  return (char *)p;
}

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
			 int *stack_recover_idx)	/* Non-speculative top-of-stack;
										 * used on mispredict recovery */
{
  struct bpred_btb_ent_t *pbtb = NULL;
  int index, i;

  if (!dir_update_ptr)
    panic("no bpred update record");

  /* if this is not a branch, return not-taken */
  if (!(MD_OP_FLAGS(op) & F_CTRL))
    return 0;

  pred->lookups++;

  dir_update_ptr->dir.ras = FALSE;
  dir_update_ptr->pdir1 = NULL;
  dir_update_ptr->pdir2 = NULL;
  dir_update_ptr->pmeta = NULL;

  if ((MD_OP_FLAGS(op) & (F_CTRL|F_UNCOND)) != (F_CTRL|F_UNCOND)) {
	dir_update_ptr->pdir1 =
	  bpred_dir_lookup (pred->bimod, baddr);
  }
    
  /*
   * We have a stateful predictor, and have gotten a pointer into the
   * direction predictor (except for jumps, for which the ptr is null)
   */

  /* record pre-pop TOS; if this branch is executed speculatively
   * and is squashed, we'll restore the TOS and hope the data
   * wasn't corrupted in the meantime. */
  if (pred->retstack.size)
    *stack_recover_idx = pred->retstack.tos;
  else
    *stack_recover_idx = 0;

  /* if this is a return, pop return-address stack */
  if (is_return && pred->retstack.size)
    {
      md_addr_t target = pred->retstack.stack[pred->retstack.tos].target;
      pred->retstack.tos = (pred->retstack.tos + pred->retstack.size - 1)
		% pred->retstack.size;
      pred->retstack_pops++;
      dir_update_ptr->dir.ras = TRUE; /* using RAS here */
      return target;
    }
  
  /* not a return. Get a pointer into the BTB */
  index = (baddr >> MD_BR_SHIFT) & (pred->btb.sets - 1);

  if (pred->btb.assoc > 1)
    {
      index *= pred->btb.assoc;

      /* Now we know the set; look for a PC match */
      for (i = index; i < (index+pred->btb.assoc) ; i++)
		if (pred->btb.btb_data[i].addr == baddr)
		  {
			/* match */
			pbtb = &pred->btb.btb_data[i];
			break;
		  }
    }	
  else
    {
      pbtb = &pred->btb.btb_data[index];
      if (pbtb->addr != baddr)
		pbtb = NULL;
    }

  /*
   * We now also have a pointer into the BTB for a hit, or NULL otherwise
   */

  /* if this is a jump, ignore predicted direction; we know it's taken. */
  if ((MD_OP_FLAGS(op) & (F_CTRL|F_UNCOND)) == (F_CTRL|F_UNCOND))
    {
      return (pbtb ? pbtb->target : 1);
    }

  /* otherwise we have a conditional branch */
  if (pbtb == NULL)
    {
      /* BTB miss -- just return a predicted direction */
      return ((*(dir_update_ptr->pdir1) >= 2)
			  ? /* taken */ 1
			  : /* not taken */ 0);
    }
  else
    {
      /* BTB hit, so return target if it's a predicted-taken branch */
      return ((*(dir_update_ptr->pdir1) >= 2)
			  ? /* taken */ pbtb->target
			  : /* not taken */ 0);
    }
}

/* Speculative execution can corrupt the ret-addr stack.  So for each
 * lookup we return the top-of-stack (TOS) at that point; a mispredicted
 * branch, as part of its recovery, restores the TOS using this value --
 * hopefully this uncorrupts the stack. */
void
bpred_recover(struct bpred_t *pred,	/* branch predictor instance */
			  int stack_recover_idx)	/* Non-speculative top-of-stack;
										 * used on mispredict recovery */
{
  if (pred == NULL) return;
  pred->retstack.tos = stack_recover_idx;
}

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
			 int correct,		/* was earlier addr prediction ok? */
			 enum md_opcode op,		/* opcode of instruction */
			 struct bpred_update_t *dir_update_ptr)/* pred state pointer */
{
  struct bpred_btb_ent_t *pbtb = NULL;
  struct bpred_btb_ent_t *lruhead = NULL, *lruitem = NULL;
  int index, i;

  /* don't change bpred state for non-branch instructions or if this
   * is a stateless predictor*/
  if (!(MD_OP_FLAGS(op) & F_CTRL)) return;

  /* Have a branch here */
  if (correct) pred->addr_hits++;

  if (!!pred_taken == !!taken) pred->dir_hits++;
  else pred->misses++;

  if (dir_update_ptr->dir.ras)
    {
      pred->used_ras++;
      if (correct)
		pred->ras_hits++;
    }
  else if ((MD_OP_FLAGS(op) & (F_CTRL|F_COND)) == (F_CTRL|F_COND))
    {
      if (dir_update_ptr->dir.meta)
		pred->used_2lev++;
      else pred->used_bimod++;
    }

  /* keep stats about JR's; also, but don't change any bpred state for JR's
   * which are returns unless there's no retstack */
  if (MD_IS_INDIR(op))
    {
      pred->jr_seen++;
      if (correct) pred->jr_hits++;
      
      if (!dir_update_ptr->dir.ras)
		{
		  pred->jr_non_ras_seen++;
		  if (correct)
			pred->jr_non_ras_hits++;
		}
	  /* return that used the ret-addr stack; no further work to do */
      else return;
    }

  /* find BTB entry if it's a taken branch (don't allocate for non-taken) */
  if (taken)
    {
      index = (baddr >> MD_BR_SHIFT) & (pred->btb.sets - 1);
      
      if (pred->btb.assoc > 1)
		{
		  index *= pred->btb.assoc;
	  
		  /* Now we know the set; look for a PC match; also identify
		   * MRU and LRU items */
		  for (i = index; i < (index+pred->btb.assoc) ; i++)
			{
			  if (pred->btb.btb_data[i].addr == baddr)
				{
				  /* match */
				  assert(!pbtb);
				  pbtb = &pred->btb.btb_data[i];
				}
	      
			  assert(pred->btb.btb_data[i].prev 
					  != pred->btb.btb_data[i].next);
			  if (pred->btb.btb_data[i].prev == NULL)
				{
				  /* this is the head of the lru list, ie current MRU item */
				  assert(lruhead == NULL);
				  lruhead = &pred->btb.btb_data[i];
				}
			  if (pred->btb.btb_data[i].next == NULL)
				{
				  /* this is the tail of the lru list, ie the LRU item */
				  assert(lruitem == NULL);
				  lruitem = &pred->btb.btb_data[i];
				}
			}
		  
		  assert(lruhead && lruitem);
		  /* missed in BTB; choose the LRU item in this set as the victim */
		  if (!pbtb) pbtb = lruitem;	
		  /* else hit, and pbtb points to matching BTB entry */
	  
		  /* Update LRU state: selected item, whether selected because it
		   * matched or because it was LRU and selected as a victim, becomes 
		   * MRU */
		  if (pbtb != lruhead)
			{
			  /* this splices out the matched entry... */
			  if (pbtb->prev) pbtb->prev->next = pbtb->next;
			  if (pbtb->next) pbtb->next->prev = pbtb->prev;
			  
			  /* ...and this puts the matched entry at the head of the list */
			  pbtb->next = lruhead;
			  pbtb->prev = NULL;
			  lruhead->prev = pbtb;
			  assert(pbtb->prev || pbtb->next);
			  assert(pbtb->prev != pbtb->next);
			}
		  /* else pbtb is already MRU item; do nothing */
		}
      else pbtb = &pred->btb.btb_data[index];
    }
      
  /* 
   * Now 'p' is a possibly null pointer into the direction prediction table, 
   * and 'pbtb' is a possibly null pointer into the BTB (either to a 
   * matched-on entry or a victim which was LRU in its set)
   */

  /* update state (but not for jumps) */

  /***************************************************************
  Perceptron training and update
  ****************************************************************/
  if (dir_update_ptr->pdir1)
    {					
	  int i;
	  signed int t, x[200];
	  int theta;
	  theta = 1.93 * (pred->bimod->config.perceptron.bhr_length) + 14;
	  int index = pred->bimod->config.perceptron.index;
	  int output = pred->bimod->config.perceptron.output;

	  if (taken) t = 1;
	  else t = -1;

	  if (output < 0) output = (-1)*output;
		
	  if (output <= theta || (output < 0 && t > 0) || (output >= 0 && t < 0))
		{							
		  for (i=0; i < pred->bimod->config.perceptron.bhr_length; i++)
			{
			  if (pred->bimod->config.perceptron.masks_table[i] == 0)
				x[i] = -1;
			  else x[i] = 1;
			  if (t == x[i])
				pred->bimod->config.perceptron.weights_table[index][i]++;
			  else 
				pred->bimod->config.perceptron.weights_table[index][i]--;
			  if (pred->bimod->config.perceptron.weights_table[index][i] > 127)
				pred->bimod->config.perceptron.weights_table[index][i] = 127;
			  if (pred->bimod->config.perceptron.weights_table[index][i] < -128)
				pred->bimod->config.perceptron.weights_table[index][i] = -128;
			}

		  for (i=1; i < pred->bimod->config.perceptron.bhr_length; i++)
			pred->bimod->config.perceptron.masks_table[i-1] = pred->bimod->config.perceptron.masks_table[i];
		  pred->bimod->config.perceptron.masks_table[pred->bimod->config.perceptron.bhr_length-1] = taken;
		}	
      
      if (taken)
		{
		  if (*dir_update_ptr->pdir1 < 3)
			++*dir_update_ptr->pdir1;
		} else
		{ /* not taken */
		  if (*dir_update_ptr->pdir1 > 0)
			--*dir_update_ptr->pdir1;
		}
   
	}
  /* combining predictor also updates second predictor and meta predictor */
  /* second direction predictor */
  if (dir_update_ptr->pdir2)
    {
      if (taken)
		{
		  if (*dir_update_ptr->pdir2 < 3)
			++*dir_update_ptr->pdir2;
		}
      else
		{ /* not taken */
		  if (*dir_update_ptr->pdir2 > 0)
			--*dir_update_ptr->pdir2;
		}
    }

  /* meta predictor */
  if (dir_update_ptr->pmeta)
    {
      if (dir_update_ptr->dir.bimod != dir_update_ptr->dir.twolev)
		{
		  /* we only update meta predictor if directions were different */
		  if (dir_update_ptr->dir.twolev == (unsigned int)taken)
			{
			  /* 2-level predictor was correct */
			  if (*dir_update_ptr->pmeta < 3)
				++*dir_update_ptr->pmeta;
			}
		  else
			{
			  /* bimodal predictor was correct */
			  if (*dir_update_ptr->pmeta > 0)
				--*dir_update_ptr->pmeta;
			}
		}
    }

  /* update BTB (but only for taken branches) */
  if (pbtb)
    {
      /* update current information */
      assert(taken);

      if (pbtb->addr == baddr)
		{
		  if (!correct)
			pbtb->target = btarget;
		}
      else
		{
		  /* enter a new branch in the table */
		  pbtb->addr = baddr;
		  pbtb->op = op;
		  pbtb->target = btarget;
		}
    }
}

/*****************************************************************
 * Printing functions for the neural branch predictor
 ****************************************************************/

/* print branch direction predictor configuration */
void
bpred_dir_config(
				 struct bpred_dir_t *pred_dir,	/* branch direction predictor instance */
				 char name[],			/* predictor name */
				 FILE *stream)			/* output stream */
{
  fprintf(stream, "pred_dir: %s:  %d perceptrons, %d weight_bits, %d history\n",
		  name, pred_dir->config.perceptron.weight_index,
		  pred_dir->config.perceptron.weight_bits,
		  pred_dir->config.perceptron.bhr_length);
}

/* print branch predictor configuration */
void
bpred_config(struct bpred_t *pred,	/* branch predictor instance */
			 FILE *stream)		/* output stream */
{

  /***************************************************************
  Setting btb associativity and return stack entries
  ****************************************************************/
  bpred_dir_config (pred->bimod, "perceptron", stream); 
  fprintf(stream, "btb: %d sets x %d associativity", 
		  pred->btb.sets, pred->btb.assoc);
  fprintf(stream, "ret_stack: %d entries", pred->retstack.size);	
}

/* print predictor stats */
void
bpred_stats(struct bpred_t *pred,	/* branch predictor instance */
			FILE *stream)		/* output stream */
{
  fprintf(stream, "pred: addr-prediction rate = %f\n",
		  (double)pred->addr_hits/(double)(pred->addr_hits+pred->misses));
  fprintf(stream, "pred: dir-prediction rate = %f\n",
		  (double)pred->dir_hits/(double)(pred->dir_hits+pred->misses));
}
