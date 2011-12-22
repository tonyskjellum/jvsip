/* Created RJudd */
/* SPAWARSYSCEN D857 */
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
/* $Id: vmprod_d.h,v 2.0 2003/02/22 15:23:29 judd Exp $ */
static void vmprod_d(void){
   printf("********\nTEST vmprod_d\n");
   {
     vsip_scalar_d datav[] = {1,2,3,4,5};
     vsip_scalar_d datam[] = { .1, .2, .3, .4,    
                              1.0, 1.1, 1.2, 1.3,
                              2.1, 2.2, 2.3, 2.4,
                              -1.1, -1.2, -1.3, -1.4,
                              2.1, 2.2, 2.3, 3.4};
     vsip_scalar_d ans_data[] = { 14.5,15.2,15.9,21.6};
     vsip_block_d *blockv = vsip_blockbind_d(datav,5,VSIP_MEM_NONE);
     vsip_block_d *blockm = vsip_blockbind_d(datam,20,VSIP_MEM_NONE);
     vsip_block_d *block_ans = vsip_blockbind_d(ans_data,4,VSIP_MEM_NONE);
     vsip_block_d *block  = vsip_blockcreate_d(70,VSIP_MEM_NONE);
     vsip_vview_d *v = vsip_vbind_d(blockv,0,1,5);
     vsip_mview_d *m = vsip_mbind_d(blockm,0,4,5,1,4);
     vsip_vview_d *ans = vsip_vbind_d(block_ans,0,1,4);
     vsip_vview_d *a = vsip_vbind_d(block,5,-1,5);
     vsip_mview_d *b = vsip_mbind_d(block,50,-2,5,-10,4);
     vsip_vview_d *c = vsip_vbind_d(block,49,-2,4);
     vsip_vview_d *chk = vsip_vcreate_d(4,VSIP_MEM_NONE);
     vsip_blockadmit_d(blockv,VSIP_TRUE);
     vsip_blockadmit_d(blockm,VSIP_TRUE);
     vsip_blockadmit_d(block_ans,VSIP_TRUE);
     vsip_vcopy_d_d(v,a);
     vsip_mcopy_d_d(m,b);
     vsip_vmprod_d(a,b,c);
     printf("vsip_vmprod_d(a,b,c)\n");    
     printf("a\n"); VU_vprintm_d("6.4",a);
     printf("b\n"); VU_mprintm_d("6.4",b);
     printf("c\n"); VU_vprintm_d("6.4",c);
     printf("right answer\n"); VU_vprintm_d("6.4",ans);
     vsip_vsub_d(c,ans,chk); vsip_vmag_d(chk,chk); vsip_vclip_d(chk,.0001,.0001,0,1,chk);
     if(vsip_vsumval_d(chk) > .5)
         printf("error\n");
     else
         printf("correct\n");
     vsip_valldestroy_d(v);
     vsip_malldestroy_d(m);
     vsip_valldestroy_d(ans);
     vsip_vdestroy_d(a);
     vsip_mdestroy_d(b);
     vsip_valldestroy_d(c);
     vsip_valldestroy_d(chk);
   }
   return;
}
