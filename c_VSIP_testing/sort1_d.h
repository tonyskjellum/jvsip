/* Created RJudd */
/**********************************************************************
// For TASP VSIPL Documentation and Code neither the United States    /
// Government, the United States Navy, nor any of their employees,    /
// makes any warranty, express or implied, including the warranties   /
// of merchantability and fitness for a particular purpose, or        /
// assumes any legal liability or responsibility for the accuracy,    /
// completeness, or usefulness of any information, apparatus,         /
// product, or process disclosed, or represents that its use would    /
// not infringe privately owned rights                                /
**********************************************************************/
/* $Id: sort1_d.h,v 1.1 2008/09/19 12:48:30 judd Exp $ */

static void sort1_d(void){
   printf("********\nTEST sort (1) for double\n");
   {
      vsip_scalar_d dta[30] = { -1, 2, 0,-3, 6, 8, 5, 4,-2, 1, 2, 3, 4, \
      5, 6, 7, 8, 9,10,11, -1,-2,-3,-4,-5, 0,-4,-5,-3,-2}; 
      vsip_scalar_d ans_dta[30] = {0, 0, 1, 1,1, 2, 2, 2,2, 2, 3, \
                                   3,3, 3, 4, 4, 4, 4, 5, 5, 5, \
                                   5, 6,  6, 7, 8, 8, 9, 10, 11};
      vsip_vview_d *s = vsip_vcreate_d(30,VSIP_MEM_NONE);
      vsip_vview_d *sinv = vsip_vcreate_d(30,VSIP_MEM_NONE);
      vsip_vview_d *ans_s = vsip_vcreate_d(30,VSIP_MEM_NONE);
      vsip_vview_vi *vi = vsip_vcreate_vi(30,VSIP_MEM_NONE);
      vsip_vview_bl *chk = vsip_vcreate_bl(30,VSIP_MEM_NONE);
      
      vsip_vcopyfrom_user_d(dta,s);
      vsip_vcopyfrom_user_d(ans_dta,ans_s);
      printf("sort by magnitude; sort ascending\n");
      vsip_vsortip_d(s, VSIP_SORT_BYMAGNITUDE, VSIP_SORT_ASCENDING,VSIP_TRUE, vi);
      vsip_vmag_d(s,s);
      vsip_vleq_d(s,ans_s,chk);
      if(vsip_valltrue_bl(chk)) printf("correct\n"); else printf("error\n"); 
      vsip_vcopyfrom_user_d(dta,s);
      vsip_vgather_d(s,vi,sinv);
      vsip_vmag_d(sinv,s);
      vsip_vleq_d(s,ans_s,chk);
      if(vsip_valltrue_bl(chk)) printf("correct\n"); else printf("error\n"); 
      
      vsip_valldestroy_vi(vi);
      vsip_valldestroy_d(s);
      vsip_valldestroy_d(sinv);
      vsip_valldestroy_d(ans_s);
   }
   return;
}
