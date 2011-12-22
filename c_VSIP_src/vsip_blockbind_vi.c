/* Created RJudd September 16, 2000 */
/* SPAWARSYSCEN D857                */
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
/* $Id: vsip_blockbind_vi.c,v 2.2 2009/05/20 17:11:15 judd Exp $ */

#include<vsip.h>
#include<vsip_blockattributes_vi.h>

vsip_block_vi* (vsip_blockbind_vi)(
  vsip_scalar_vi* const p,
  size_t N, 
  vsip_memory_hint h) {
  vsip_block_vi* b = (vsip_block_vi*)malloc(sizeof(vsip_block_vi));
  { 
    if(b != (vsip_block_vi*)NULL){
       b->kind     = VSIP_USER_BLOCK;
       b->admit    = VSIP_RELEASED_BLOCK;
       b->markings = VSIP_VALID_STRUCTURE_OBJECT;
       b->array    = p;   /* user data array        */
       b->size     = N;   /* size in block elements */
       b->bindings = 0;
       b->hint     = h;
    }
  }
  return b;
}
