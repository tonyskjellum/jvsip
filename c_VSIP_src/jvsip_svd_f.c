/*
 //  jvsip_svd_f.c
 //  jvsipF
 //
 //  Created by RANDALL JUDD on 4/21/13.
 //  Copyright (c) 2013 Independent Consultant. All rights reserved.
 */
/*********************************************************************
 // This code includes no warranty, express or implied, including     /
 // the warranties of merchantability and fitness for a particular    /
 // purpose.                                                          /
 // No person or entity assumes any legal liability or responsibility /
 // for the accuracy, completeness, or usefulness of any information, /
 // apparatus, product, or process disclosed, or represents that      /
 // its use would not infringe privately owned rights.                /
 *********************************************************************/
/*********************************************************************
 // The MIT License (see copyright for jvsip in top level directory)
 // http://opensource.org/licenses/MIT
 **********************************************************************/
#include<vsip.h>
#include<vsip_svdattributes_f.h>
#include"VI.h"
#include"vsip_blockattributes_f.h"
#include"vsip_cblockattributes_f.h"
#include"vsip_blockattributes_vi.h"
#include"vsip_vviewattributes_f.h"
#include"vsip_mviewattributes_f.h"
#include"vsip_cvviewattributes_f.h"
#include"vsip_cmviewattributes_f.h"
#include"vsip_vviewattributes_vi.h"

#define VI_VGET_F(v,i) (*(v->block->array + (v->offset + (vsip_stride)(i) * v->stride) * v->block->rstride))

#define scaleV(v) { \
vsip_index k;vsip_scalar_f *p = v->block->array + v->offset * v->block->rstride;\
vsip_stride std = v->stride * v->block->rstride;vsip_length n = v->length * std;\
vsip_scalar_f scl = p[0];p[0]=1.0;for(k=std; k<n; k+=std) p[k] /= scl; \
}

#define CMAG_F(re,im) ((re == 0.0) ? im:(im< re) ? re * (vsip_scalar_f)sqrt(1.0 + (im/re) * (im/re)): \
im * (vsip_scalar_f)sqrt(1.0 + (re/im) * (re/im)))

/* coded for this routine. Only done in place */
#define csvmul_f(alpha, b) {\
vsip_length _n = b->length; vsip_stride cbst = b->block->cstride;\
vsip_scalar_f *bpr = b->block->R->array + cbst * b->offset,*bpi = b->block->I->array + cbst * b->offset;\
register vsip_scalar_f temp; vsip_stride bst = cbst * b->stride;\
while(_n-- > 0){temp = alpha.r * *bpr - *bpi * alpha.i;*bpi = alpha.r * *bpi + alpha.i * *bpr;*bpr = temp;\
bpr += bst; bpi += bst;}}

#define cvcopy_f(a, r) {\
    vsip_length _n =  r->length;vsip_stride cast = a->block->cstride,crst = r->block->cstride;\
    vsip_scalar_f *apr = a->block->R->array + cast * a->offset,*rpr = r->block->R->array + crst * r->offset;\
    vsip_scalar_f *api = a->block->I->array + cast * a->offset,*rpi = r->block->I->array + crst * r->offset;\
    vsip_stride ast = (cast * a->stride),rst = (crst * r->stride);\
    while(_n-- > 0){*rpr = *apr;*rpi = *api;apr += ast; api += ast;rpr += rst; rpi += rst;}}

#define vcopy_f(a,r) {vsip_length _n   = r->length;\
    vsip_stride ast = a->stride * a->block->rstride,rst = r->stride * r->block->rstride;\
    vsip_scalar_f *ap = a->block->array + a->offset * a->block->rstride,*rp = r->block->array+r->offset * r->block->rstride;\
    while(_n-- > 0){*rp = *ap;ap += ast; rp += rst;}}

#define vcopy_vi(a,r) {vsip_length _n   = r->length;\
    vsip_stride ast = a->stride,rst = r->stride;\
    vsip_scalar_vi *ap = a->block->array + a->offset,*rp = r->block->array+r->offset;\
    while(_n-- > 0){*rp = *ap;ap += ast; rp += rst;}}

#define mcopy_f(a,r) {vsip_length n_mj,n_mn;vsip_stride ast_mj, ast_mn,rst_mj, rst_mn;\
    vsip_scalar_f *rp = (r->block->array) + r->offset * r->block->rstride,\
    *ap = (a->block->array) + a->offset * a->block->rstride;vsip_scalar_f *rp0 = rp,*ap0 = ap;\
    if(r->row_stride < r->col_stride){n_mj   = r->row_length; n_mn = r->col_length;\
    rst_mj = r->row_stride; rst_mn = r->col_stride;ast_mj = a->row_stride; ast_mn = a->col_stride;\
    rst_mj *= r->block->rstride; rst_mn *= r->block->rstride;ast_mj *= a->block->rstride; ast_mn *= a->block->rstride;\
    } else {n_mn = r->row_length; n_mj = r->col_length;\
    rst_mn = r->row_stride; rst_mj = r->col_stride;ast_mn = a->row_stride; ast_mj = a->col_stride;\
    rst_mn *= r->block->rstride; rst_mj *= r->block->rstride;ast_mn *= a->block->rstride; ast_mj *= a->block->rstride;\
    } while(n_mn-- > 0){vsip_length n = n_mj;while(n-- >0){*rp = *ap;ap += ast_mj;  rp += rst_mj;\
    }ap0 += ast_mn; rp0 += rst_mn;ap = ap0; rp = rp0;}}

#define cmcopy_f(a,r) {vsip_length n_mj, n_mn; vsip_stride ast_mj, ast_mn,rst_mj, rst_mn; \
    vsip_scalar_f *ap_r = a->block->R->array + a->offset * a->block->cstride,*rp_r = r->block->R->array + r->offset * r->block->cstride;\
    vsip_scalar_f *ap_i = a->block->I->array + a->offset * a->block->cstride,*rp_i = r->block->I->array + r->offset * r->block->cstride;\
    vsip_scalar_f *ap0_r = ap_r,*rp0_r = rp_r,*ap0_i = ap_i,*rp0_i = rp_i;\
    if(r->row_stride < r->col_stride){n_mj = r->row_length; n_mn = r->col_length;\
    rst_mj = r->row_stride; rst_mn = r->col_stride;ast_mj = a->row_stride; ast_mn = a->col_stride;\
    rst_mj *= r->block->cstride; rst_mn *= r->block->cstride;ast_mj *= a->block->cstride; ast_mn *= a->block->cstride;\
    } else {n_mn = r->row_length; n_mj = r->col_length;\
    rst_mn = r->row_stride; rst_mj = r->col_stride;ast_mn = a->row_stride; ast_mj = a->col_stride;\
    rst_mn *= r->block->cstride; rst_mj *= r->block->cstride;ast_mn *= a->block->cstride; ast_mj *= a->block->cstride;}\
    while(n_mn-- > 0){vsip_length n = n_mj;while(n-- >0){*rp_r = *ap_r;*rp_i = *ap_i;ap_r += ast_mj;  rp_r += rst_mj;\
    ap_i += ast_mj;  rp_i += rst_mj;}ap0_r += ast_mn; rp0_r += rst_mn;ap0_i += ast_mn; rp0_i += rst_mn;\
    ap_r = ap0_r; rp_r = rp0_r; ap_i = ap0_i; rp_i = rp0_i;}}

#define cmherm_f(_A,_R) {	\
    vsip_length  lx = _A->row_length,ly = _A->col_length;vsip_stride cAst = _A->block->cstride,cRst = _R->block->cstride;\
    vsip_scalar_f *a_p_r = _A->block->R->array + cAst * _A->offset,\
    *a_p_i = _A->block->I->array + cAst * _A->offset,*r_p_r = _R->block->R->array + cRst * _R->offset, *r_p_i;vsip_length i, j;\
    vsip_stride stAx = cAst * _A->row_stride, stAy = cAst *_A->col_stride, stRx = cRst * _R->row_stride,stRy = cRst *_R->col_stride;\
    for(i=0; i<ly; i++){\
    r_p_r = _R->block->R->array + cRst * _R->offset + i * stRx;r_p_i = _R->block->I->array + cRst * _R->offset + i * stRx;\
    a_p_r = _A->block->R->array + cAst * _A->offset + i * stAy;a_p_i = _A->block->I->array + cAst * _A->offset + i * stAy;\
    for(j=0; j<lx; j++){*r_p_r = *a_p_r;*r_p_i = - *a_p_i;r_p_r += stRy; a_p_r += stAx;r_p_i += stRy; a_p_i += stAx;}}}

#define cmconj_f( a, r) { vsip_length n_mj, n_mn; vsip_stride ast_mj, ast_mn, rst_mj, rst_mn; \
    vsip_scalar_f *ap_r = a->block->R->array + a->offset * a->block->cstride, *rp_r = r->block->R->array + r->offset * r->block->cstride, \
    *ap_i = a->block->I->array + a->offset * a->block->cstride, *rp_i = r->block->I->array + r->offset * r->block->cstride, \
    *ap0_r = ap_r, *rp0_r = rp_r, *ap0_i = ap_i, *rp0_i = rp_i; \
    if(r->row_stride < r->col_stride){ n_mj = r->row_length; n_mn = r->col_length; \
    rst_mj = r->row_stride; rst_mn = r->col_stride; ast_mj = a->row_stride; ast_mn = a->col_stride; \
    rst_mj *= r->block->cstride; rst_mn *= r->block->cstride; ast_mj *= a->block->cstride; ast_mn *= a->block->cstride; \
    } else { n_mn = r->row_length; n_mj = r->col_length; \
    rst_mn = r->row_stride; rst_mj = r->col_stride; ast_mn = a->row_stride; ast_mj = a->col_stride; \
    rst_mn *= r->block->cstride; rst_mj *= r->block->cstride; ast_mn *= a->block->cstride; ast_mj *= a->block->cstride; \
    } while(n_mn-- > 0){ vsip_length _n = n_mj; while(_n-- >0){ \
    *rp_r = *ap_r; *rp_i = - *ap_i; ap_r += ast_mj; rp_r += rst_mj; ap_i += ast_mj; rp_i += rst_mj; \
    } ap0_r += ast_mn; rp0_r += rst_mn; ap_r = ap0_r;    rp_r = rp0_r; ap0_i += ast_mn; rp0_i += rst_mn; ap_i = ap0_i;rp_i = rp0_i;}}

#define mpermute_onceCol_f(in, p){ vsip_scalar_f *ptr,t;vsip_length n_dta, n_ind; vsip_stride in_dta_strd, in_ind_strd; \
    register vsip_index to0; register vsip_index from0; vsip_index _i,_j,*b,from,to;\
    n_dta = in->col_length; n_ind = in->row_length; in_dta_strd = in->col_stride * in->block->rstride;\
    in_ind_strd = in->row_stride * in->block->rstride; ptr = in->block->array + in->offset * in->block->rstride;\
    b = p->block->array + p->offset; for(_i=0; _i<n_ind; _i++){ vsip_index r_or_c = b[_i * p->stride];\
    from0 = b[_i * p->stride] * in_ind_strd; to0 = _i * in_ind_strd;\
    if(from0 != to0) { for(_j=0; _j<n_dta; _j++){to = to0 + _j * in_dta_strd, from = from0 + _j * in_dta_strd;\
    t = ptr[to]; ptr[to] = ptr[from]; ptr[from] = t; } _j=_i;\
    while(_i != b[_j * p->stride]){ _j++; if(_j > n_ind) exit(-1); } b[_j * p->stride] = r_or_c;}}}

#define mpermute_onceRow_f(in, p) { vsip_index _i,_j,from,to; vsip_length n_dta, n_ind; \
    vsip_stride in_dta_strd=in->row_stride * in->block->rstride,in_ind_strd=in->col_stride * in->block->rstride;\
    vsip_scalar_f *ptr = in->block->array + in->offset * in->block->rstride,t;\
    vsip_scalar_vi *b = p->block->array + p->offset; n_dta = in->row_length; n_ind = in->col_length;\
    for(_i=0; _i<n_ind; _i++){ vsip_index r_or_c = b[_i * p->stride]; \
    register vsip_index from0 = b[_i * p->stride] * in_ind_strd; register vsip_index to0 = _i * in_ind_strd;\
    if(from0 != to0) { for(_j=0; _j<n_dta; _j++){ to = to0 + _j * in_dta_strd; from = from0 + _j * in_dta_strd;\
    t = ptr[to]; ptr[to] = ptr[from]; ptr[from] = t; } _j=_i; while(_i != b[_j * p->stride])\
    { _j++; if(_j > n_ind) exit(-1);}b[_j * p->stride] = r_or_c; } } }

#define cmpermute_onceCol_f(in, p){\
    vsip_length n_dta=in->col_length, n_ind=in->row_length; vsip_stride in_dta_strd, in_ind_strd; vsip_index _i,_j;\
    vsip_scalar_f *ptr_re = in->block->R->array + in->offset * in->block->cstride, \
    *ptr_im = in->block->I->array + in->offset * in->block->cstride;\
    vsip_scalar_vi *b = p->block->array + p->offset;\
    in_dta_strd = in->col_stride * in->block->cstride; in_ind_strd = in->row_stride * in->block->cstride;\
    for(_i=0; _i<n_ind; _i++){vsip_index r_or_c = b[_i * p->stride]; \
    register vsip_index from0 = b[_i * p->stride] * in_ind_strd; register vsip_index to0 = _i * in_ind_strd;\
    if(from0 != to0) { for(_j=0; _j<n_dta; _j++){ vsip_index to = to0 + _j * in_dta_strd; \
    vsip_index from = from0 + _j * in_dta_strd;vsip_scalar_f t = ptr_re[to]; ptr_re[to] = ptr_re[from];\
    ptr_re[from] = t; t = ptr_im[to];ptr_im[to] = ptr_im[from]; ptr_im[from] = t;\
    }_j=_i; while(_i != b[_j * p->stride]){ _j++; if(_j > n_ind) exit(-1); } b[_j * p->stride] = r_or_c; } } }

#define cmpermute_onceRow_f( in, p){\
    vsip_length n_dta, n_ind; vsip_stride in_dta_strd, in_ind_strd; vsip_index _i,_j;\
    vsip_scalar_f *ptr_re = in->block->R->array + in->offset * in->block->cstride, \
    *ptr_im = in->block->I->array + in->offset * in->block->cstride;\
    vsip_scalar_vi *b = p->block->array + p->offset; n_dta = in->row_length; n_ind = in->col_length;\
    in_dta_strd = in->row_stride * in->block->cstride; in_ind_strd = in->col_stride * in->block->cstride;\
    for(_i=0; _i<n_ind; _i++){ vsip_index r_or_c = b[_i * p->stride]; \
    register vsip_index from0 = b[_i * p->stride] * in_ind_strd; register vsip_index to0 = _i * in_ind_strd;\
    if(from0 != to0) { for(_j=0; _j<n_dta; _j++){ vsip_index to = to0 + _j * in_dta_strd; \
    vsip_index from = from0 + _j * in_dta_strd;vsip_scalar_f t = ptr_re[to]; ptr_re[to] = ptr_re[from]; \
    ptr_re[from] = t; t = ptr_im[to]; ptr_im[to] = ptr_im[from]; ptr_im[from] = t;\
    }_j=_i; while(_i != b[_j * p->stride]){ _j++; if(_j > n_ind) exit(-1); } b[_j * p->stride] = r_or_c;}}}

#define vmprod_f(a,B,r) {vsip_length nx=0,mx=0;\
    vsip_scalar_f *ap=a->block->array+a->offset * a->block->rstride,*ap0=ap,\
    *rp=r->block->array+r->offset * r->block->rstride, *Byp=B->block->array+B->offset * B->block->rstride,*Bxp = Byp;\
    vsip_stride BCst = B->col_stride * B->block->rstride,BRst=B->row_stride * B->block->rstride,\
    rst  = r->stride * r->block->rstride; while(nx++ < B->row_length){*rp=0;mx=0;\
    while(mx++ < B->col_length){ *rp += *ap * *Byp; ap += a->stride; Byp += BCst; }ap = ap0; Byp = (Bxp += BRst); rp += rst;}}

#define mvprod_f(A, b, r) {vsip_length nx=0, mx=0;\
    vsip_scalar_f  *bp=b->block->array+b->offset * b->block->rstride,\
    *rp=r->block->array+r->offset * r->block->rstride, *Ayp=A->block->array+A->offset * A->block->rstride, *Axp=Ayp;\
    vsip_stride rst=r->stride * r->block->rstride, ARst=A->row_stride * A->block->rstride,\
    ACst=A->col_stride * A->block->rstride, bst=b->stride * b->block->rstride;\
    while(nx++ < A->col_length){ *rp=0; mx=0; while(mx++ < A->row_length){ *rp += *bp * *Axp; bp += bst; Axp += ARst; }\
    bp=b->block->array+b->offset * b->block->rstride; Axp=(Ayp += ACst); rp += rst; } }

#define svmul_f(alpha, b, r) {vsip_length n = r->length;\
    vsip_stride bst=b->stride * b->block->rstride, rst = r->stride * r->block->rstride;\
    vsip_scalar_f *bp=b->block->array+b->offset * b->block->rstride, *rp=r->block->array+r->offset * r->block->rstride;\
    while(n-- > 0){ *rp = alpha * *bp; bp += bst; rp += rst;}}

#define mtrans_f(A, R) { vsip_length  lx = A->row_length, ly = A->col_length; vsip_index i, j; vsip_scalar_f tmp;\
    vsip_scalar_f *a_p = A->block->array + A->offset * A->block->rstride, *r_p = R->block->array + R->offset * R->block->rstride;\
    vsip_stride stAx = A->row_stride * A->block->rstride, stAy = A->col_stride * A->block->rstride,\
    stRx = R->row_stride * R->block->rstride, stRy = R->col_stride * R->block->rstride;\
    if((lx == ly) && (a_p == r_p)){ for(i=1; i<lx; i++){ for(j=0; j<i; j++){ tmp = *(a_p + j * stAy + i * stAx);\
    *(a_p + j * stAy + i * stAx) = *(a_p + j * stAx + i * stAy); *(a_p + j * stAx + i * stAy) = tmp; } }\
    } else { for(i=0; i<ly; i++){ r_p = R->block->array + R->offset * R->block->rstride + i * stRx;\
    a_p = A->block->array + A->offset * A->block->rstride + i * stAy; for(j=0; j<lx; j++){ *r_p = *a_p;\
    r_p += stRy; a_p += stAx; } } } }

typedef struct {vsip_index i; vsip_index j;} svdCorner;
typedef struct {vsip_scalar_f c; vsip_scalar_f s; vsip_scalar_f r;}givensObj_f ;

static svdObj_f *svdInit_f(vsip_length, vsip_length);
static void svdFinalize_f(svdObj_f*);
static void svd_f(svdObj_f*);

static csvdObj_f *csvdInit_f(vsip_length, vsip_length);
static void csvdFinalize_f(csvdObj_f*);
static void csvd_f(csvdObj_f*);
static vsip_scalar_f hypot_f(vsip_scalar_f a0,vsip_scalar_f b0){
    vsip_scalar_f a = (a0 < 0.0) ? -a0:a0;
    vsip_scalar_f b = (b0 < 0.0) ? -b0:b0;
    if (a == 0.0)
        return b;
    else if (b == 0.0)
        return a;
    else if (b < a)
        return a * (vsip_scalar_f)sqrt(1.0 + (b/a) * (b/a));
    else
        return b * (vsip_scalar_f)sqrt(1.0 + (a/b) * (a/b));
}

static vsip_vview_f *vsv_f(vsip_vview_f *v, vsip_vview_f *vs, vsip_index i)
{
    *vs=*v;
    vs->offset += i * vs->stride;
    vs->length -= i;
    return vs;
}
static vsip_mview_f* msv_f(vsip_mview_f *B,vsip_mview_f *BS, vsip_index i,vsip_index j)
{
    *BS = *B;
    BS->row_length -= j;
    BS->col_length -= i;
    BS->offset += j * BS->row_stride + i * BS->col_stride;
    return BS;
}
static vsip_mview_f* imsv_f( vsip_mview_f *B, vsip_mview_f *BS, vsip_index i1,vsip_index j1,
                            vsip_index i2, vsip_index j2)
{
    *BS=*B;
    if(j1 == 0) j1 =B->col_length;
    if(j2 == 0) j2 =B->row_length;
    BS->col_length = (j1 - i1);
    BS->row_length = (j2 - i2);
    BS->offset += i2 * B->row_stride + i1 * B->col_stride;
    return BS;
}
static vsip_vview_f *ivsv_f( vsip_vview_f *v, vsip_vview_f *vs, vsip_index i,vsip_index j)
{
    *vs=*v;
    if(j==0) j=v->length;
    vs->offset += i * v->stride;
    vs->length = j-i;
    return vs;
}
static vsip_vview_f *col_sv_f(vsip_mview_f*Am,vsip_vview_f* vv,vsip_index col)
{
    vv->block = Am->block;
    vv->offset = Am->offset + col * Am->row_stride;
    vv->stride = Am->col_stride;
    vv->length = Am->col_length;
    return vv;
}
static vsip_vview_f *row_sv_f(vsip_mview_f*Am,vsip_vview_f* vv,vsip_index row)
{
    vv->block = Am->block;
    vv->offset = Am->offset + row * Am->col_stride;
    vv->stride = Am->row_stride;
    vv->length = Am->row_length;
    return vv;
}
static vsip_vview_f *diag_sv_f(vsip_mview_f* Am,vsip_vview_f* a, vsip_stride i)
{
    a->block = Am->block;
    a->stride=Am->row_stride + Am->col_stride;
    if(i==0){
        a->length = Am->row_length;
        a->offset = Am->offset;
    } else if (i == 1){
        a->offset = Am->offset + Am->row_stride;
        a->length = Am->row_length - 1;
    } else {
        printf("Failed in diag_sv_f\n");
        exit(0);
    }
    return a;
}

static vsip_cmview_f* cimsv_f( vsip_cmview_f *B, vsip_cmview_f *BS, vsip_index i1,vsip_index j1,
                              vsip_index i2, vsip_index j2)
{
    *BS=*B;
    if(j1 == 0) j1 =B->col_length;
    if(j2 == 0) j2 =B->row_length;
    BS->col_length = (j1 - i1);
    BS->row_length = (j2 - i2);
    BS->offset += i2 * B->row_stride + i1 * B->col_stride;
    return BS;
}
static vsip_cvview_f *ccol_sv_f(vsip_cmview_f*Am,vsip_cvview_f* vv,vsip_index col)
{
    vv->block = Am->block;
    vv->offset = Am->offset + col * Am->row_stride;
    vv->stride = Am->col_stride;
    vv->length = Am->col_length;
    return vv;
}
static vsip_cvview_f *crow_sv_f(vsip_cmview_f*Am,vsip_cvview_f* vv,vsip_index row)
{
    vv->block=Am->block;
    vv->offset = Am->offset + row * Am->col_stride;
    vv->stride = Am->row_stride;
    vv->length = Am->row_length;
    return vv;
}
static vsip_cvview_f *cdiag_sv_f(vsip_cmview_f* Am,vsip_cvview_f* a, vsip_stride i)
{
    a->block = Am->block;
    a->stride=Am->row_stride + Am->col_stride;
    if(i==0){
        a->length = Am->row_length;
        a->offset = Am->offset;
    } else if (i == 1){
        a->offset = Am->offset + Am->row_stride;
        a->length = Am->row_length - 1;
    } else {
        printf("Failed in diag_sv_f\n");
        exit(0);
    }
    return a;
}

static vsip_scalar_f vnorm2_f(vsip_vview_f *a)
{
    vsip_length n = a->length;
    vsip_stride ast = a->stride * a->block->rstride;
    vsip_scalar_f *ap = (a->block->array) + a->offset * a->block->rstride,t = 0;
    while(n-- > 0){
        t  += (*ap * *ap);
        ap += ast;
    }
    return (vsip_scalar_f)sqrt(t);
}
static vsip_scalar_f mnormFro_f(vsip_mview_f *v)
{
    vsip_length m=v->col_length,n=v->row_length;
    vsip_index i,j;
    vsip_stride rstrd=v->row_stride*v->block->rstride;
    vsip_stride cstrd=v->col_stride*v->block->rstride;
    vsip_scalar_f *vre=v->block->array+v->offset*v->block->rstride;
    vsip_scalar_f re=0.0;
    for(i=0; i<m; i++){
        vsip_offset o=i*cstrd;
        vsip_scalar_f *rp=vre+o;
        for(j=0; j<n; j++){
            re += *rp * *rp;
            rp += rstrd;
        }
    }
    return sqrt(re);
}
static void meye_f(vsip_mview_f *v)
{
    vsip_length m=v->col_length,n=v->row_length;
    vsip_index i,j;
    vsip_stride rstrd=v->row_stride*v->block->rstride;
    vsip_stride cstrd=v->col_stride*v->block->rstride;
    vsip_scalar_f *vre=v->block->array+v->offset*v->block->rstride;
    for(i=0; i<m; i++){
        vsip_offset o=i*cstrd;
        vsip_scalar_f *rp=vre+o;
        for(j=0; j<n; j++){
            *rp = (i==j) ? 1.0:0.0;
            rp += rstrd;
        }
    }
}

static void svdZeroCheckAndSet_f(vsip_scalar_f e, vsip_vview_f *d, vsip_vview_f *f)
{
    vsip_index i;
    vsip_length n = f->length;
    vsip_stride fstd=f->stride*f->block->rstride;
    vsip_stride dstd=d->stride*d->block->rstride;
    vsip_scalar_f *fptr=f->block->array+f->offset*f->block->rstride;
    vsip_scalar_f *dptr=d->block->array+d->offset*d->block->rstride;
    for(i=0; i<n; i++){
        vsip_scalar_f *fi=fptr+i*fstd,*di=dptr+i*fstd;
        if((*fi<0 ? -*fi:*fi) < (e*(*di<0 ? -*di:*di + *(di+dstd)<0 ? -*(di+dstd):*(di+dstd)))) *fi=0.0;
    }
}

/* sign function as defined in http://www.netlib.org/lapack/lawnspdf/lawn148.pdf */
static vsip_scalar_f sign_f(vsip_scalar_f a_in)
{
    return (a_in < 0.0) ? -1.0:1.0;
}
static vsip_cscalar_f csign_f(vsip_cscalar_f a_in)
{
    vsip_scalar_f re = a_in.r < 0.0 ? -a_in.r:a_in.r;
    vsip_scalar_f im = a_in.i < 0.0 ? -a_in.i:a_in.i;
    vsip_scalar_f t= (re==0.0) ? im: ((im==0.0) ? re:((re<im) ? im*sqrt(1.0 + re/im*re/im):re*sqrt(1.0+im/re*im/re)));
    vsip_cscalar_f retval;
    retval.r=0.0; retval.i = 0.0;
    if(t == 0.0){
        retval.r = 1.0;
        return retval;
    } else if (im==0.0) {
        retval.r = (a_in.r < 0.0) ? -1.0:1.0;
        return retval;
    } else {
        retval.r = a_in.r/t;
        retval.i = a_in.i/t;
        return retval;
    }
}

static void phaseCheck_f(svdObj_f *svd)
{
    vsip_scalar_f *fptr, *dptr;
    vsip_mview_f *L = svd->L;
    vsip_vview_f *d = svd->d;
    vsip_vview_f *f = svd->f;
    vsip_mview_f *R = svd->R;
    vsip_scalar_f eps0 = svd->eps0;
    vsip_length n_d=d->length;
    vsip_length n_f=f->length;
    vsip_index i,j;
    vsip_scalar_f ps;
    vsip_scalar_f m;
    vsip_vview_f *l = &svd->ls_one;
    vsip_vview_f *r = &svd->rs_one;
    vsip_stride fstrd=f->stride * d->block->rstride;
    vsip_stride dstrd=d->stride * d->block->rstride;
    dptr=d->block->array +d->offset * d->block->rstride;
    fptr=f->block->array + f->offset * f->block->rstride;
    for(i=0; i<n_d; i++){
        ps=*dptr;
        m = (ps<0) ? -ps:ps;
        ps=sign_f(ps);
        if(m > eps0){
            col_sv_f(L,l,i);
            {
                vsip_scalar_f *p = l->block->array + l->offset * l->block->rstride;
                vsip_stride std = l->stride * l->block->rstride;
                vsip_length n = l->length * std;
                vsip_index k;
                for(k=0; k<n; k+=std) p[k] *= ps;
            }
            *dptr=m;
            if (i < n_f)
                *fptr *= ps;
        } else {
            *dptr = 0.0;
        }
        dptr+=dstrd;fptr+=fstrd;
    }
    svdZeroCheckAndSet_f(eps0,d,f);
    fptr=f->block->array + f->offset * f->block->rstride;
    for (i=0; i<n_f-1; i++){
        j=i+1;
        ps = *fptr;
        m = (ps<0) ? -ps:ps;
        ps=sign_f(ps);
        *fptr=m;
        col_sv_f(L, l, j);
        {
            vsip_scalar_f *p = l->block->array + l->offset * l->block->rstride;
            vsip_stride std = l->stride * l->block->rstride;
            vsip_length n = l->length * std;
            vsip_index k;
            for(k=0; k<n; k+=std) p[k] *= ps;
        }
        row_sv_f(R,r,j);
        {
            vsip_scalar_f *p = r->block->array + r->offset * r->block->rstride;
            vsip_stride std = r->stride * r->block->rstride;
            vsip_length n = r->length * std;
            vsip_index k;
            for(k=0; k<n; k+=std) p[k] *= ps;
        }
        fptr+=fstrd;
        *fptr *= ps;
    }
    j=n_f;
    ps=*fptr;
    m=(ps<0) ? -ps:ps;
    ps=sign_f(ps);
    *fptr=m;
    col_sv_f(L, l, j);
    {
        vsip_scalar_f *p = l->block->array + l->offset * l->block->rstride;
        vsip_stride std = l->stride * l->block->rstride;
        vsip_length n = l->length * std;
        vsip_index k;
        for(k=0; k<n; k+=std) p[k] *= ps;
    }
    row_sv_f(R,r,j);
    {
        vsip_scalar_f *p = r->block->array + r->offset * r->block->rstride;
        vsip_stride std = r->stride * r->block->rstride;
        vsip_length n = r->length * std;
        vsip_index k;
        for(k=0; k<n; k+=std) p[k] *= ps;
    }
}

static void biDiagPhaseToZero_f( svdObj_f *svd)
{
    vsip_vview_f *x=&svd->bs,*v=svd->d;
    diag_sv_f(svd->B,x,0);
    vcopy_f(x,v);

    diag_sv_f(svd->B,x,1); v=svd->f;
    vcopy_f(x,v);
    
    phaseCheck_f(svd);
}

#define opu_f(A,x,y) {\
    vsip_length m = A->col_length,n = A->row_length;vsip_index i,ii,j,jj;\
    vsip_scalar_f *ap=A->block->array + A->offset * A->block->rstride, \
    *xp=x->block->array + x->offset * x->block->rstride,*yp=y->block->array + y->offset * y->block->rstride;\
    vsip_stride xstd = x->block->rstride * x->stride,ystd = y->block->rstride * y->stride,\
    r_std = A->block->rstride * A->row_stride,c_std = A->block->rstride * A->col_stride;\
    m *= c_std; n *= r_std;\
    for(i=0, ii=0; i<m; i+=c_std, ii+=xstd){vsip_scalar_f c = xp[ii], *app=ap+i;\
    for(j=0, jj=0; j<n; j+=r_std, jj+=ystd){ app[j] = app[j] + c * yp[jj];}}}

static void houseProd_f(vsip_vview_f *v, vsip_mview_f *A,vsip_vview_f *w)
{
    vsip_scalar_f *vptr = v->block->array + v->offset*v->block->rstride;
    vsip_stride vstd = v->stride*v->block->rstride;
    vsip_scalar_f beta = 0.0;
    vsip_length n=v->length;
    while(n-- > 0){ beta += *vptr * *vptr;vptr+=vstd;}
    beta=2.0/beta;
    w->length = A->row_length;
    vmprod_f(v,A,w)
    svmul_f(-beta,w,w)
    opu_f(A,v,w)
}
static void prodHouse_f(vsip_mview_f *A, vsip_vview_f *v, vsip_vview_f *w)
{
    vsip_scalar_f *vptr = v->block->array + v->offset*v->block->rstride;
    vsip_stride vstd = v->stride*v->block->rstride;
    vsip_scalar_f beta = 0.0;
    vsip_length n=v->length;
    while(n-- > 0){ beta += *vptr * *vptr;vptr+=vstd;}
    beta=2.0/beta;
    w->length = A->col_length;
    mvprod_f(A,v,w)
    svmul_f(-beta,w,w)
    opu_f(A,w,v)
}
static vsip_vview_f *houseVector_f(vsip_vview_f* x)
{
    vsip_scalar_f *p=x->block->array + x->offset * x->block->rstride;
    vsip_scalar_f nrm=vnorm2_f(x);
    vsip_scalar_f t = *p;
    vsip_scalar_f s = t +  sign_f(t) * nrm;
    *p=s;
    nrm = vnorm2_f(x);
    if (nrm == 0.0)
        *p=1.0;
    else {
        vsip_stride std = x->stride * x->block->rstride;
        vsip_length n = x->length * std;
        vsip_index k;
        for(k=0; k<n; k+=std) p[k] /= nrm;
    }
    return x;
}
static void VHmatExtract_f(svdObj_f *svd)
{
    vsip_mview_f*B = svd->B;
    vsip_index i,j;
    vsip_length n = B->row_length;
    vsip_mview_f *Bs=&svd->Bs;
    vsip_mview_f *V=svd->R;
    vsip_mview_f *Vs=&svd->Rs;
    vsip_vview_f *v=&svd->bs;
    vsip_scalar_f t;
    vsip_scalar_f *vptr;
    if(n < 3)
        return;
    for(i=n-3; i>0; i--){
        j=i+1;
        row_sv_f(msv_f(B,Bs,i,j),v,0);
        vptr=v->block->array+v->offset*v->block->rstride;
        t=*vptr; *vptr = 1.0;
        prodHouse_f(msv_f(V,Vs,j,j),v,svd->w);
        *vptr = t;
    }
    row_sv_f(msv_f(B,Bs,0,1),v,0);
    vptr=v->block->array+v->offset*v->block->rstride;
    t=*vptr; *vptr = 1.0;
    prodHouse_f(msv_f(V,Vs,1,1),v,svd->w);
    *vptr = t;
}
static void UmatExtract_f(svdObj_f *svd)
{
    vsip_mview_f* B=svd->B;
    vsip_mview_f *U=svd->L;
    vsip_index i;
    vsip_length m = B->col_length;
    vsip_length n = B->row_length;
    vsip_mview_f *Bs=&svd->Bs;
    vsip_mview_f *Us=&svd->Ls;
    vsip_vview_f *v=&svd->bs;
    vsip_scalar_f t;
    vsip_scalar_f *vptr;
    if (m > n){
        i=n-1;
        col_sv_f(msv_f(B,Bs,i,i),v,0);
        vptr=v->block->array+v->offset*v->block->rstride;
        t=*vptr; *vptr = 1.0;
        houseProd_f(v,msv_f(U,Us,i,i),svd->w);
        *vptr = t;
    }
    for(i=n-2; i>0; i--){
        col_sv_f(msv_f(B,Bs,i,i),v,0);
        vptr=v->block->array+v->offset*v->block->rstride;
        t=*vptr; *vptr = 1.0;
        houseProd_f(v,msv_f(U,Us,i,i),svd->w);
        *vptr = t;
    }
    col_sv_f(msv_f(B,Bs,0,0),v,0);
    vptr=v->block->array+v->offset*v->block->rstride;
    t=*vptr; *vptr = 1.0;
    houseProd_f(v,msv_f(U,Us,0,0),svd->w);
    *vptr= t;
}

static void bidiag_f(svdObj_f *svd)
{
    vsip_mview_f *B = svd->B;
    vsip_mview_f *Bs = &svd->Bs;
    vsip_length m = B->col_length;
    vsip_length n = B->row_length;
    vsip_vview_f *x=col_sv_f(B,&svd->bs,0);
    vsip_vview_f *v=svd->t;
    vsip_vview_f *vs = &svd->ts;
    vsip_index i,j;
    for(i=0; i<n-1; i++){
        v->length=m-i;
        col_sv_f(msv_f(B,Bs,i,i),x,0);
        vcopy_f(x,v);/*0*/
        houseVector_f(v);
        scaleV(v)
        houseProd_f(v,Bs,svd->w);
        vsv_f(v,vs,1);vsv_f(x,x,1);
        vcopy_f(vs,x);/*1*/
        if(i < n-2){
            j = i+1;
            v->length = n-j;
            row_sv_f(msv_f(B,Bs,i,j),x,0);
            vcopy_f(x,v);/*2*/
            houseVector_f(v);
            scaleV(v)
            prodHouse_f(Bs,v,svd->w);
            vsv_f(v,vs,1);vsv_f(x,x,1);
            vcopy_f(vs,x);/*3*/
        }
    }
    if(m > n){
        i=n-1;
        v->length=m-i;
        col_sv_f(msv_f(B,Bs,i,i),x,0);
        vcopy_f(x,v); /*4*/
        houseVector_f(v);
        scaleV(v)
        houseProd_f(v,Bs,svd->w);
        vsv_f(v,vs,1);vsv_f(x,x,1);
        vcopy_f(vs,x);/*5*/
    }
}
static void svdBidiag_f(svdObj_f *svd)
{
    vsip_mview_f *B = svd->B;
    /* eps0 is a number << maximum singular value */
    svd->eps0=(mnormFro_f(B)/(vsip_scalar_f)B->row_length)*FLT_EPSILON;/* 1E-10;*/
    bidiag_f(svd);
    UmatExtract_f(svd);
    VHmatExtract_f(svd);
    biDiagPhaseToZero_f(svd);
}

static void gtProd_f(vsip_index i, vsip_index j, vsip_scalar_f c,vsip_scalar_f s, svdObj_f* svd)
{
    vsip_mview_f* R = &svd->Rs;
    vsip_vview_f *a1= row_sv_f(R,&svd->rs_one, i);
    vsip_vview_f *a2= row_sv_f(R,&svd->rs_two, j);
    vsip_index k;
    vsip_scalar_f *t1 = a1->block->array + (a1->offset) * a1->block->rstride;
    vsip_scalar_f *t2 = a2->block->array + (a2->offset) * a2->block->rstride;
    vsip_stride std = a1->stride * a1->block->rstride;
    vsip_length n = a1->length * std;
    for(k=0; k<n; k+=std){
        register vsip_scalar_f b1 = t1[k];
        register vsip_scalar_f b2 = t2[k];
        t1[k] = c * b1 + s * b2;
        t2[k] = c * b2 - s * b1;
    }
}
static void prodG_f(svdObj_f* svd,vsip_index i, vsip_index j,vsip_scalar_f c, vsip_scalar_f s)
{
    vsip_mview_f* L = &svd->Ls;
    vsip_vview_f *a1= col_sv_f(L,&svd->ls_one,i);
    vsip_vview_f *a2= col_sv_f(L,&svd->ls_two,j);
    vsip_index k;
    vsip_scalar_f *t1 = a1->block->array + (a1->offset) * a1->block->rstride;
    vsip_scalar_f *t2 = a2->block->array + (a2->offset) * a2->block->rstride;
    vsip_stride std = a1->stride * a1->block->rstride;
    vsip_length n = a1->length * std;
    for(k=0; k<n; k+=std){
        register vsip_scalar_f b1 = t1[k];
        register vsip_scalar_f b2 = t2[k];
        t1[k] = c * b1 + s * b2;
        t2[k] = c * b2 - s * b1;
    }
}
static givensObj_f givensCoef_f(vsip_scalar_f x1, vsip_scalar_f x2)
{
    givensObj_f retval;
    vsip_scalar_f t = hypot_f(x1,x2);
    if (x2 == 0.0 ){
        retval.c=1.0;retval.s=0.0;retval.r=x1;
    } else if (x1 == 0.0  || t == 0.0) {
        retval.c=0.0;retval.s=sign_f(x2);retval.r=t;
    }else{
        vsip_scalar_f sn = sign_f(x1);
        retval.c=((x1 < 0.0) ? -x1:x1)/t;retval.s=sn*x2/t; retval.r=sn*t;
    }
    return retval;
} /* same */

static void zeroCol_f(svdObj_f *svd)
{
    vsip_vview_f *d=&svd->ds;
    vsip_vview_f *f=&svd->fs;
    vsip_length n = f->length;
    givensObj_f g;
    vsip_scalar_f xd,xf,t;
    vsip_index i,j,k;
    if (n == 1){
        xd=*(d->block->array + d->offset * d->block->rstride);
        xf=*(f->block->array + f->offset * f->block->rstride);
        g=givensCoef_f(xd,xf);
        *(d->block->array + d->offset * d->block->rstride)=g.r;
        *(f->block->array + f->offset * f->block->rstride)=0.0;
        gtProd_f(0,1,g.c,g.s,svd);
    }else if (n == 2){
        xd=VI_VGET_F(d,1);
        xf=VI_VGET_F(f,1);
        g=givensCoef_f(xd,xf);
        *(d->block->array + (d->offset+1) * d->block->rstride)=g.r;
        *(f->block->array + (f->offset+1) * f->block->rstride)=0.0;
        xf=VI_VGET_F(f,0);
        t= -xf * g.s; xf *= g.c;
        *(f->block->array + (f->offset) * f->block->rstride)=xf;
        gtProd_f(1,2,g.c,g.s,svd);
        xd=VI_VGET_F(d,0);
        g=givensCoef_f(xd,t);
        *(d->block->array + d->offset * d->block->rstride)=g.r;
        gtProd_f(0,2,g.c,g.s,svd);
    }else{
        i=n-1; j=i-1; k=i;
        xd=*(d->block->array + (d->offset+i) * d->block->rstride);
        xf=*(f->block->array + (f->offset+i) * f->block->rstride);
        g=givensCoef_f(xd,xf);
        xf=*(f->block->array + (f->offset+j) * f->block->rstride);
        *(d->block->array + (d->offset+i) * d->block->rstride)=g.r;
        *(f->block->array + (f->offset+i) * f->block->rstride)=0.0;
        t=-xf*g.s; xf*=g.c;
        *(f->block->array + (f->offset+j) * f->block->rstride)=xf;
        gtProd_f(i,k+1,g.c,g.s,svd);
        while (i > 1){
            i = j; j = i-1;
            xd=*(d->block->array + (d->offset+i) * d->block->rstride);
            g=givensCoef_f(xd,t);
            *(d->block->array + (d->offset+i) * d->block->rstride)=g.r;
            xf=*(f->block->array + (f->offset+j) * f->block->rstride);
            t= -xf * g.s; xf *= g.c;
            *(f->block->array + (f->offset+j) * f->block->rstride)=xf;
            gtProd_f(i,k+1,g.c,g.s,svd);
        }
        xd=*(d->block->array + d->offset * d->block->rstride);
        g=givensCoef_f(xd,t);
        *(d->block->array + d->offset * d->block->rstride)=g.r;
        gtProd_f(0,k+1,g.c,g.s,svd);
    }
}
static void zeroRow_f(svdObj_f *svd)
{
    vsip_vview_f *d = &svd->ds;
    vsip_vview_f *f = &svd->fs;
    vsip_scalar_f *dptr=d->block->array+d->offset*d->block->rstride;
    vsip_scalar_f *fptr=f->block->array + f->offset*d->block->rstride;
    vsip_stride dstd=d->stride*d->block->rstride;
    vsip_stride fstd=f->stride*f->block->rstride;
    vsip_length n = d->length;
    givensObj_f g;
    vsip_scalar_f xd,xf,t;
    vsip_index i;
    xd=*dptr;
    xf=*fptr;
    g=givensCoef_f(xd,xf);
    if (n == 1){
        *dptr=g.r;
        *fptr=0.0;
        prodG_f(svd,n,0,g.c,g.s);
    }else{
        *dptr=g.r;
        *fptr=0.0;
        xf=*(fptr+fstd);
        t= -xf * g.s; xf *= g.c;
        *(fptr+fstd)=xf;
        prodG_f(svd,1,0,g.c,g.s);
        for(i=1; i<n-1; i++){
            xd=*(dptr+dstd*i);
            g=givensCoef_f(xd,t);
            prodG_f(svd,i+1,0,g.c,g.s);
            *(dptr+dstd*i)=g.r;
            xf=*(fptr+fstd*(i+1));
            t=-xf * g.s; xf *= g.c;
            *(fptr+fstd*(i+1))=xf;
        }
        xd=*(dptr+dstd*(n-1));
        g=givensCoef_f(xd,t);
        *(dptr+dstd*(n-1))=g.r;
        prodG_f(svd,n,0,g.c,g.s);
    }
}

static vsip_scalar_f svdMu_f(vsip_scalar_f d2,vsip_scalar_f f1,vsip_scalar_f d3,vsip_scalar_f* f2)
{
    vsip_scalar_f mu;
    vsip_scalar_f cu=d2 * d2 + f1 * f1;
    vsip_scalar_f cl=d3 * d3 + *f2 * *f2;
    vsip_scalar_f cd = d2 * *f2;
    vsip_scalar_f T = (cu + cl);
    vsip_scalar_f D = (cu * cl - cd * cd)/(T*T);
    vsip_scalar_f root = T * (vsip_scalar_f) sqrt(1.0 - ((4 * D)>1.0 ? 1.0:4*D));
    vsip_scalar_f lambda1 = (T + root)/(2.);
    vsip_scalar_f lambda2 = (T - root)/(2.);
    vsip_scalar_f c1=lambda1-cl,c2=lambda2-cl;
    c1=(c1<0) ? -c1:c1;  c2=(c2<0) ? -c2:c2;
    if(root == 0.0)
        if(*f2 < (d2 + d3)/1E6)
            *f2=0.0;
    if(c1 < c2)
        mu = lambda1;
    else
        mu = lambda2;
    return mu;
}
static vsip_index zeroFind_f(vsip_vview_f* d, vsip_scalar_f eps0)
{
    vsip_scalar_f *dptr=d->block->array + d->offset*d->block->rstride;
    vsip_stride dstrd=d->stride*d->block->rstride;
    vsip_index j = d->length;
    vsip_scalar_f xd=*(dptr+(j-1)*dstrd);
    while(xd > eps0){
        if (j > 1){
            j -= 1;
            xd=*(dptr+(j-1)*dstrd);
        }else if(j==1)
            return 0;
    }
    *(dptr+(j-1)*dstrd)=0.0;
    return j;
}
static svdCorner svdCorners_f(vsip_vview_f* f)
{
    svdCorner crnr;
    vsip_scalar_f *fptr=f->block->array + f->offset*f->block->rstride;
    vsip_stride fstrd=f->stride*f->block->rstride;
    vsip_index j=f->length-1;
    vsip_index i;
    while((j > 0) && *(fptr+j*fstrd) == 0.0)
        j-=1;
    if(j == 0 && *fptr == 0.0){
        crnr.i=0;
        crnr.j=0;
    } else {
        i = j;
        while((i > 0) && *(fptr+i*fstrd) != 0.0)
            i -= 1;
        if(i==0 && *fptr == 0.0){
            crnr.i = 1;
            crnr.j = j+2;
        } else if(i == 0 && *fptr != 0.0){
            crnr.i = 0;
            crnr.j= j+2;
            
        }else{
            crnr.i = i+1;
            crnr.j = j+2;
        }
    } /* index and length of f != 0 */
    return crnr;
}

static void svdStep_f(svdObj_f *svd)
{
    vsip_vview_f *d = &svd->ds;
    vsip_vview_f *f = &svd->fs;
    givensObj_f g;
    vsip_length n = d->length;
    vsip_scalar_f mu=0.0, x1=0.0, x2=0.0;
    vsip_scalar_f t=0.0;
    vsip_index i,j,k;
    vsip_scalar_f d2=0.0,f1=0.0,d3=0.0,f2=0.0;
    vsip_scalar_f *fptr,*dptr;
    vsip_stride fstd=f->stride*f->block->rstride, dstd=d->stride*d->block->rstride;
    dptr=d->block->array+d->offset*d->block->rstride;
    fptr=f->block->array+f->offset*f->block->rstride;
    if(n >= 3){
        d2=*(dptr+dstd*(n-2));f1= *(fptr+fstd*(n-3));d3 = *(dptr+dstd*(n-1));f2= *(fptr+fstd*(n-2));
    } else if(n == 2){
        d2=*dptr; d3=*(dptr+dstd); f1=0; f2=*fptr;
    } else {
        printf("should not be here (see svdStep_f)\n");
        exit(-1);
    }
    mu = svdMu_f(d2,f1,d3,&f2);
    if(f2 == 0.0) *(fptr+fstd*(n-2)) = 0.0;
    x1=*dptr;
    x2 = x1 * *fptr;
    x1 *= x1; x1 -= mu;
    g=givensCoef_f(x1,x2);
    x1=*dptr;x2=*fptr;
    *fptr=g.c*x2-g.s*x1;
    *dptr=g.c*x1+g.s*x2;
    t=*(dptr+dstd);
    *(dptr+dstd) *= g.c;
    t*=g.s;
    gtProd_f(0,1,g.c,g.s,svd);
    for(i=0; i<n-2; i++){
        j=i+1; k=i+2;
        g = givensCoef_f(*(dptr+i*dstd),t);
        *(dptr+i*dstd)=g.r;
        x1=*(dptr+j*dstd)*g.c;
        x2=*(fptr+i*fstd)*g.s;
        t= x1 - x2;
        x1=*(fptr+i*fstd) * g.c;
        x2=*(dptr+j*dstd) * g.s ;
        *(f->block->array+(i*f->stride+f->offset)*f->block->rstride) = x1+x2;
        *(d->block->array+(j*d->stride+d->offset)*d->block->rstride) = t;
        x1=*(fptr+j*fstd);
        t=g.s * x1;
        *(fptr+j*fstd)=x1*g.c;
        prodG_f(svd,i, j, g.c, g.s);
        g=givensCoef_f(*(fptr+i*fstd),t);
        *(fptr+i*fstd)=g.r;
        x1=*(dptr+j*dstd); x2=*(fptr+j*fstd);
        *(dptr+j*dstd)=g.c * x1 + g.s * x2; *(fptr+j*fstd)=g.c * x2 - g.s * x1;
        x1=*(dptr+k*dstd);
        t=g.s * x1; *(dptr+k*dstd)=x1*g.c;
        gtProd_f(j,k, g.c, g.s,svd);
    }
    i=n-2; j=n-1;
    g = givensCoef_f(*(dptr+i*dstd),t);
    *(d->block->array+(i*d->stride+d->offset)*d->block->rstride) = g.r;
    x1=*(dptr+j*dstd)*g.c; x2=*(fptr+i*fstd)*g.s;
    t=x1 - x2;
    x1 = *(fptr+i*fstd) * g.c; x2=*(dptr+j*dstd) * g.s;
    *(f->block->array+(i*f->stride+f->offset)*f->block->rstride) = x1+x2;
    *(d->block->array+(j*d->stride+d->offset)*d->block->rstride) = t;
    prodG_f(svd,i, j, g.c, g.s);
}
static void svdIteration_f(svdObj_f* svd)
{
    vsip_mview_f *L0 = svd->L; vsip_mview_f *L = &svd->Ls;
    vsip_vview_f *d0 = svd->d; vsip_vview_f *d = &svd->ds;
    vsip_vview_f *f0 = svd->f; vsip_vview_f *f = &svd->fs;
    vsip_mview_f *R0 = svd->R; vsip_mview_f *R= &svd->Rs;
    vsip_scalar_f eps0 = svd->eps0;
    vsip_length n;
    svdCorner cnr;
    vsip_index k;
    vsip_length cntr=0;
    vsip_length maxcntr=5*d0->length;
    while (cntr++ < maxcntr){
        phaseCheck_f(svd);
        cnr=svdCorners_f(f0);
        if (cnr.j == 0)
            break;
        ivsv_f(d0,d,cnr.i,cnr.j);
        ivsv_f(f0,f,cnr.i,cnr.j-1);
        imsv_f(L0,L,0,0,cnr.i,cnr.j);
        imsv_f(R0,R,cnr.i,cnr.j,0,0);
        n=f->length;
        k=zeroFind_f(d,eps0);
        if (k > 0 ){
            k=k-1;
            if(VI_VGET_F(d,n) == 0.0){
                zeroCol_f(svd);           
            }else{
                imsv_f(L,L,0,0,k,0);
                d->length-=k+1;
                d->offset += k+1;
                f->length -= k;
                f->offset += k;
                zeroRow_f(svd);
            }
        }else{
            svdStep_f(svd);
        }
        
    }
#ifdef DEBUG
    printf("maxcounter %lu, count %lu\n",maxcntr,cntr);
#endif
}

static vsip_cvview_f *cvsv_f(vsip_cvview_f *v,vsip_cvview_f *vs,vsip_index i)
{
    *vs=*v;
    vs->offset += i * vs->stride;
    vs->length -= i;
    return vs;
}
static vsip_cmview_f* cmsv_f(vsip_cmview_f *B, vsip_cmview_f *BS, vsip_index i,vsip_index j)
{
    *BS=*B;
    BS->row_length -= j;
    BS->col_length -= i;
    BS->offset += j * BS->row_stride + i * BS->col_stride;
    return BS;
}

static vsip_vview_f *vreal_sv_f(vsip_cvview_f *cv,vsip_vview_f*v)
{
    v->block=cv->block->R;
    v->offset = cv->offset; v->length=cv->length;v->stride= cv->stride;
    v->markings = VSIP_VALID_STRUCTURE_OBJECT;
    return v;
}

static vsip_scalar_f cvnorm2_f(vsip_cvview_f *a)
{
    vsip_length n = a->length;
    vsip_stride cast = a->block->cstride;
    vsip_scalar_f *apr = (vsip_scalar_f*) ((a->block->R->array) + cast * a->offset);
    vsip_scalar_f *api = (vsip_scalar_f*) ((a->block->I->array) + cast * a->offset);
    vsip_stride ast = (cast * a->stride);
    vsip_scalar_f t1=0,t2=0;
    while(n-- > 0){
        t1 += *apr * *apr;
        t2 += *api * *api;
        apr += ast; api += ast;
    }
    return (vsip_scalar_f)sqrt(t1+t2);
}

static vsip_scalar_f cmnormFro_f(vsip_cmview_f *v)
{
    vsip_length m=v->col_length,n=v->row_length;
    vsip_index i,j;
    vsip_stride rstrd=v->row_stride*v->block->R->rstride;
    vsip_stride cstrd=v->col_stride*v->block->R->rstride;
    vsip_scalar_f *vre=v->block->R->array+v->offset*v->block->R->rstride;
    vsip_scalar_f *vim=v->block->I->array+v->offset*v->block->I->rstride;
    vsip_scalar_f re=0.0;
    vsip_scalar_f im=0.0;
    for(i=0; i<m; i++){
        vsip_offset o=i*cstrd;
        vsip_scalar_f *rp=vre+o;
        vsip_scalar_f *ip=vim+o;
        for(j=0; j<n; j++){
            re += *rp * *rp;
            im += *ip * *ip;
            rp += rstrd; ip += rstrd;
        }
    }
    return (vsip_scalar_f)sqrt(re+im);
}
static void cmeye_f(vsip_cmview_f *v)
{
    vsip_length m=v->col_length,n=v->row_length;
    vsip_index i,j;
    vsip_stride rstrd=v->row_stride*v->block->R->rstride;
    vsip_stride cstrd=v->col_stride*v->block->R->rstride;
    vsip_scalar_f *vre=v->block->R->array+v->offset*v->block->R->rstride;
    vsip_scalar_f *vim=v->block->I->array+v->offset*v->block->I->rstride;
    for(i=0; i<m; i++){
        vsip_offset o=i*cstrd;
        vsip_scalar_f *rp=vre+o;
        vsip_scalar_f *ip=vim+o;
        for(j=0; j<n; j++){
            *rp = (i==j) ? 1.0:0.0;
            *ip = 0.0;
            rp += rstrd; ip += rstrd;
        }
    }
}

static void csvdZeroCheckAndSet_f(vsip_scalar_f e, vsip_cvview_f *d, vsip_cvview_f *f)
{
    vsip_index i;
    vsip_stride std = f->stride * f->block->R->rstride;
    vsip_length n = f->length * std;
    vsip_scalar_f *f_i = f->block->I->array + f->offset * f->block->R->rstride;
    vsip_scalar_f *f_r = f->block->R->array + f->offset * f->block->R->rstride;
    vsip_scalar_f *d_i = d->block->I->array + d->offset * d->block->R->rstride;
    vsip_scalar_f *d_r = d->block->R->array + d->offset * d->block->R->rstride;
    vsip_scalar_f re_f, re_d, im_f, im_d,a0,a1,a,b;
    re_d=fabs(*d_r); im_d=fabs(*d_i);
    a0=CMAG_F(re_d,im_d);
    d_r+=std; d_i+=std;
    for(i=0; i<n; i+=std){
        re_f=fabs(f_r[i]); im_f=fabs(f_i[i]);
        re_d=fabs(d_r[i]), im_d=fabs(d_i[i]);
        b = CMAG_F(re_f,im_f);
        a1=CMAG_F(re_d,im_d);
        a = e*(a0+a1);a0=a1;
        if( b < a ){ *f_r = 0.0; *f_i = 0.0;}
    }
}
static void cbiDiagPhaseToZero_f( csvdObj_f *svd)
{
    vsip_cmview_f *L = svd->L;
    vsip_cvview_f *d = cdiag_sv_f(svd->B,&svd->bs,0);
    vsip_cvview_f *f = cdiag_sv_f(svd->B,&svd->bfs,1);
    vsip_cmview_f *R = svd->R;
    vsip_scalar_f eps0 = svd->eps0;
    vsip_length n_d=d->length;
    vsip_length n_f=f->length;
    vsip_index i,j;
    vsip_cscalar_f ps;
    vsip_scalar_f m;
    vsip_cvview_f *l = &svd->ls_one;
    vsip_cvview_f *r = &svd->rs_one;
    vsip_scalar_f *dptr_r=d->block->R->array+d->block->R->rstride*d->offset;
    vsip_scalar_f *dptr_i=d->block->I->array+d->block->R->rstride*d->offset;
    vsip_scalar_f *fptr_r=f->block->R->array+f->block->R->rstride*f->offset;
    vsip_scalar_f *fptr_i=f->block->I->array+f->block->R->rstride*f->offset;
    vsip_stride dstrd=d->stride*d->block->R->rstride;
    vsip_stride fstrd=f->stride*f->block->R->rstride;
    vsip_scalar_f re,im;
    for(i=0; i<n_d; i++){
        ps.r=*(dptr_r+i*dstrd);
        ps.i=*(dptr_i+i*dstrd);
        if(ps.i == 0.0){
            m = ps.r;
            if (m < 0.0)
                ps.r=-1.0;
            else
                ps.r=1.0;
            m = (m<0) ? -m:m;
        } else {
            re=fabs(ps.r), im=fabs(ps.i);
            m = CMAG_F(re,im);
            ps.r /= m; ps.i/=m;
        }
        if(m > eps0){
            ccol_sv_f(L,l,i);csvmul_f(ps,l);
            *(d->block->R->array+(d->offset+i*d->stride)*d->block->cstride)=m;
            *(d->block->I->array+(d->offset+i*d->stride)*d->block->cstride)=0.0;
            if (i < n_f){
                vsip_scalar_f *fr=fptr_r+i*fstrd;
                vsip_scalar_f *fi=fptr_i+i*fstrd;
                re=*fr,im=*fi;
                *fr=re*ps.r+im*ps.i;
                *fi=-ps.i*re+ps.r*im;
            }
        }else{
            *(dptr_r+i*dstrd)=0.0;
            *(dptr_i+i*dstrd)=0.0;
        }
    }
    csvdZeroCheckAndSet_f(eps0,d,f);
    for (i=0; i<n_f-1; i++){
        j=i+1;
        ps.r=*(fptr_r+i*fstrd);
        ps.i=*(fptr_i+i*fstrd);
        if (ps.i == 0.0){
            m = ps.r;
            if (m < 0.0)
                ps.r =-1.0;
            else
                ps.r = 1.0;
            m = (m < 0.0) ? -m:m;
        }else{
            re=fabs(ps.r), im=fabs(ps.i);
            m = CMAG_F(re,im);
            ps.r /= m; ps.i/=m;
        }
        ccol_sv_f(L, l, j);
        ps.i=-ps.i;csvmul_f(ps,l);ps.i=-ps.i;
        crow_sv_f(R,r,j);
        csvmul_f(ps,r);
        *(fptr_r+i*fstrd)=m;
        *(fptr_i+i*fstrd)=0.0;
        re=*(fptr_r+ j * fstrd);
        im=*(fptr_i+ j * fstrd);
        *(fptr_r+ j * fstrd)=re*ps.r-im*ps.i;
        *(fptr_i+ j * fstrd)=re*ps.i+im*ps.r;
    }
    j=n_f;
    i=j-1;
    ps.r=*(fptr_r+i*fstrd);
    ps.i=*(fptr_i+i*fstrd);
    if (ps.i == 0.0){
        m = ps.r;
        if(m < 0.0)
            ps.r =-1.0;
        else
            ps.r = 1.0;
        m = (m<0) ? -m:m;
    }else{
        re=fabs(ps.r), im=fabs(ps.i);
        m = CMAG_F(re,im);
        ps.r /= m; ps.i/=m;
    }
    *(fptr_r+i*fstrd)=m;
    *(fptr_i+i*fstrd)=0.0;
    ccol_sv_f(L, l, j);
    ps.i=-ps.i;csvmul_f(ps,l);ps.i=-ps.i;
    crow_sv_f(R,r,j);
    csvmul_f(ps,r);
}
static void cphaseCheck_f(csvdObj_f *svd)
{
    vsip_cmview_f *L = svd->L;
    vsip_vview_f *d = svd->d;
    vsip_vview_f *f = svd->f;
    vsip_cmview_f *R = svd->R;
    vsip_scalar_f eps0 = svd->eps0;
    vsip_length nf=f->length;
    vsip_index i,j,k;
    vsip_scalar_f ps;
    vsip_scalar_f m;
    vsip_cvview_f *l = &svd->ls_one;
    vsip_cvview_f *r = &svd->rs_one;
    vsip_scalar_f*fptr=f->block->array;
    vsip_scalar_f*dptr=d->block->array;
    vsip_scalar_f *re,*im;
    vsip_stride strd;
    for(i=0; i<d->length; i++){
        ps=dptr[i];
        m = (ps<0) ? -ps:ps;
        ps=sign_f(ps);
        if(m > eps0){
            ccol_sv_f(L,l,i);
            strd=l->stride*l->block->R->rstride;
            re=l->block->R->array+l->offset*l->block->R->rstride;
            im=l->block->I->array+l->offset*l->block->R->rstride;
            for(k=0; k<l->length*strd; k+=strd){
                re[k]*=ps;
                im[k]*=ps;
            }
            dptr[i]=m;
            if (i < nf)
                fptr[i]*=ps;
        } else {
            dptr[i]=0.0;
        }
    }
    svdZeroCheckAndSet_f(eps0,d,f);
    for (i=0; i<nf-1; i++){
        j=i+1;
        ps = fptr[i];
        m = (ps<0) ? -ps:ps;
        ps=sign_f(ps);
        ccol_sv_f(L, l, j);
        strd=l->stride*l->block->R->rstride;
        re=l->block->R->array+l->offset*l->block->R->rstride;
        im=l->block->I->array+l->offset*l->block->R->rstride;
        for(k=0; k<l->length*strd; k+=strd){
            re[k]*=ps;
            im[k]*=ps;
        }
        crow_sv_f(R,r,j);
        strd=r->stride*r->block->R->rstride;
        re=r->block->R->array+r->offset*r->block->R->rstride;
        im=r->block->I->array+r->offset*r->block->R->rstride;
        for(k=0; k<r->length*strd; k+=strd){
            re[k]*=ps;
            im[k]*=ps;
        }
        fptr[i]=m;
        fptr[j]*=ps;
    }
    j=nf;
    i=j-1;
    ps=fptr[i];
    m=(ps<0) ? -ps:ps;
    ps=sign_f(ps);
    fptr[i]=m;
    ccol_sv_f(L, l, j);
    strd=l->stride*l->block->R->rstride;
    re=l->block->R->array+l->offset*l->block->R->rstride;
    im=l->block->I->array+l->offset*l->block->R->rstride;
    for(k=0; k<l->length*strd; k+=strd){
        re[k]*=ps;
        im[k]*=ps;
    }
    crow_sv_f(R,r,j);
    strd=r->stride*r->block->R->rstride;
    re=r->block->R->array+r->offset*r->block->R->rstride;
    im=r->block->I->array+r->offset*r->block->R->rstride;
    for(k=0; k<r->length*strd; k+=strd){
        re[k]*=ps;
        im[k]*=ps;
    }
}
static void chouseProd_f(vsip_cvview_f *v, vsip_cmview_f *A, vsip_cvview_f*w)
{
    vsip_scalar_f *Aptr_r = A->block->R->array+A->offset * A->block->R->rstride;
    vsip_scalar_f *Aptr_i = A->block->I->array+A->offset * A->block->R->rstride;
    vsip_scalar_f *vptr_r = v->block->R->array+v->offset * v->block->R->rstride;
    vsip_scalar_f *vptr_i = v->block->I->array+v->offset * v->block->R->rstride;
    vsip_scalar_f *wptr_r = w->block->R->array+w->offset * w->block->R->rstride;
    vsip_scalar_f *wptr_i = w->block->I->array+w->offset * w->block->R->rstride;
    vsip_stride vstrd=v->stride * v->block->R->rstride;
    vsip_stride wstrd=w->stride * w->block->R->rstride;
    vsip_stride AcStrd=A->col_stride*A->block->R->rstride;
    vsip_stride ArStrd=A->row_stride*A->block->R->rstride;
    vsip_index i,j,vi,wi;
    vsip_length M = A->col_length * A->col_stride * A->block->R->rstride;
    vsip_length N = A->row_length * A->row_stride * A->block->R->rstride;
    vsip_scalar_f beta = 0.0;
    vsip_scalar_f t1=0,t2=0;
    for(i=0;i<v->length;i++){
        vsip_scalar_f re=vptr_r[i*vstrd],im=vptr_i[i*vstrd];
        t1 += re*re;
        t2 += im*im;
    }
    beta=2.0/(t1+t2);
    w->length = A->row_length;
    for(i=0,wi=0; i<N; i+=ArStrd,wi+=wstrd){
        vsip_scalar_f *ar_r=&Aptr_r[i], *ar_i=&Aptr_i[i];
        vsip_scalar_f re=0.0,im=0.0;
        for(j=0,vi=0; j<M; j+=AcStrd, vi+=vstrd){
            re +=  ar_r[j] * vptr_r[vi] + ar_i[j] * vptr_i[vi];
            im += -ar_r[j] * vptr_i[vi] + ar_i[j] * vptr_r[vi];
        }
        wptr_r[wi] = re;
        wptr_i[wi] = -im;
    }
    for(i=0, vi=0; i<M; i+=AcStrd, vi+=vstrd){
        vsip_scalar_f cr = beta * vptr_r[vi];
        vsip_scalar_f ci = beta * vptr_i[vi];
        vsip_scalar_f *aprp=Aptr_r+i;
        vsip_scalar_f *apip=Aptr_i+i;
        for(j=0, wi=0; j<N; j+=ArStrd, wi+=wstrd){
            vsip_scalar_f yr =  wptr_r[wi];
            vsip_scalar_f yi = -wptr_i[wi];
            aprp[j] -= (cr * yr - ci * yi);
            apip[j] -= (cr * yi + ci * yr);
        }
    }
}
static void cprodHouse_f(vsip_cmview_f *A, vsip_cvview_f *v, vsip_cvview_f *w)
{
    vsip_scalar_f *Aptr_r = A->block->R->array+A->offset * A->block->R->rstride;
    vsip_scalar_f *Aptr_i = A->block->I->array+A->offset * A->block->R->rstride;
    vsip_scalar_f *vptr_r = v->block->R->array+v->offset * v->block->R->rstride;
    vsip_scalar_f *vptr_i = v->block->I->array+v->offset * v->block->R->rstride;
    vsip_scalar_f *wptr_r = w->block->R->array+w->offset * w->block->R->rstride;
    vsip_scalar_f *wptr_i = w->block->I->array+w->offset * w->block->R->rstride;
    vsip_stride vstrd=v->stride * v->block->R->rstride;
    vsip_stride wstrd=w->stride * w->block->R->rstride;
    vsip_stride AcStrd=A->col_stride*A->block->R->rstride;
    vsip_stride ArStrd=A->row_stride*A->block->R->rstride;
    vsip_index i,j,vi,wi;
    vsip_length M = A->col_length * A->col_stride * A->block->R->rstride;
    vsip_length N = A->row_length * A->row_stride * A->block->R->rstride;
    vsip_scalar_f beta = 0.0;
    vsip_scalar_f t1=0,t2=0;
    for(i=0;i<v->length;i++){
        vsip_scalar_f re=vptr_r[i*vstrd],im=vptr_i[i*vstrd];
        t1 += re*re;
        t2 += im*im;
    }
    beta=2.0/(t1+t2);

    w->length = A->col_length;
    for(i=0,wi=0; i<M; i+=AcStrd,wi+=wstrd){
        vsip_scalar_f *ac_r=&Aptr_r[i], *ac_i=&Aptr_i[i];
        vsip_scalar_f re=0.0,im=0.0;
        for(j=0,vi=0; j<N; j+=ArStrd, vi+=vstrd){
            re += ac_r[j] * vptr_r[vi] - ac_i[j] * vptr_i[vi];
            im += ac_r[j] * vptr_i[vi] + ac_i[j] * vptr_r[vi];
        }
        wptr_r[wi] = -beta * re;
        wptr_i[wi] = -beta * im;
    }
    for(i=0, wi=0; i<M; i+=AcStrd, wi+=wstrd){
        vsip_scalar_f cr = wptr_r[wi];
        vsip_scalar_f ci = wptr_i[wi];
        vsip_scalar_f *aprp=Aptr_r+i;
        vsip_scalar_f *apip=Aptr_i+i;
        for(j=0, vi=0; j<N; j+=ArStrd, vi+=vstrd){
            vsip_scalar_f yr =  vptr_r[vi];
            vsip_scalar_f yi = -vptr_i[vi];
            aprp[j] += (cr * yr - ci * yi);
            apip[j] += (cr * yi + ci * yr);
        }
    }
}
static vsip_cvview_f *chouseVector_f(vsip_cvview_f* x)
{
    vsip_scalar_f *x0r=x->block->R->array + x->offset * x->block->R->rstride;
    vsip_scalar_f *x0i=x->block->I->array + x->offset * x->block->R->rstride;
    vsip_scalar_f nrm=cvnorm2_f(x);
    vsip_cscalar_f t,s;
    t.r=*x0r;t.i=*x0i;
    s=csign_f(t);
    s.r *= nrm; s.i *= nrm;
    s.r += t.r; s.i +=t.i;
    *x0r = s.r;*x0i=s.i;
    nrm = cvnorm2_f(x);
    if (nrm == 0.0){
        *x0r=1.0;*x0i=0.0;
    }else{
        vsip_index i;
        vsip_stride str=x->stride * x->block->R->rstride;
        vsip_length n = x->length * str;
        for(i=0; i<n; i+=str){
            x0r[i] /= nrm;
            x0i[i] /= nrm;
        }
    }
    return x;
}
static void cVHmatExtract_f(csvdObj_f *svd)
{
    vsip_cmview_f*B = svd->B;
    vsip_index i,j;
    vsip_length n = B->row_length;
    vsip_cmview_f *Bs=&svd->Bs;
    vsip_cmview_f *V=svd->R;
    vsip_cmview_f *Vs=&svd->Rs;
    vsip_cvview_f *v=&svd->bs;
    vsip_cscalar_f t;
    if(n < 3)
        return;
    for(i=n-3; i>0; i--){
        j=i+1;
        crow_sv_f(cmsv_f(B,Bs,i,j),v,0);
        t.r=*(v->block->R->array+v->offset*v->block->cstride);
        t.i=*(v->block->I->array+v->offset*v->block->cstride);
        *(v->block->R->array+v->offset*v->block->cstride)=1.0;
        *(v->block->I->array+v->offset*v->block->cstride)=0.0;
        cprodHouse_f(cmsv_f(V,Vs,j,j),v,svd->w);
        *(v->block->R->array+v->offset*v->block->cstride)=t.r;
        *(v->block->I->array+v->offset*v->block->cstride)=t.i;
    }
    crow_sv_f(cmsv_f(B,Bs,0,1),v,0);
    t.r=*(v->block->R->array+v->offset*v->block->cstride);
    t.i=*(v->block->I->array+v->offset*v->block->cstride);
    *(v->block->R->array+v->offset*v->block->cstride)=1.0;
    *(v->block->I->array+v->offset*v->block->cstride)=0.0;
    cprodHouse_f(cmsv_f(V,Vs,1,1),v,svd->w);
    *(v->block->R->array+v->offset*v->block->cstride)=t.r;
    *(v->block->I->array+v->offset*v->block->cstride)=t.i;
}
static void cUmatExtract_f(csvdObj_f *svd)
{
    vsip_cmview_f* B=svd->B;
    vsip_cmview_f* U=svd->L;
    vsip_index i;
    vsip_length m = B->col_length;
    vsip_length n = B->row_length;
    vsip_cmview_f *Bs=&svd->Bs;
    vsip_cmview_f *Us=&svd->Ls;
    vsip_cvview_f *v=&svd->bs;
    vsip_cscalar_f t;
    if (m > n){
        i=n-1;
        ccol_sv_f(cmsv_f(B,Bs,i,i),v,0);
        t.r=*(v->block->R->array+v->offset*v->block->cstride);
        t.i=*(v->block->I->array+v->offset*v->block->cstride);
        *(v->block->R->array+v->offset*v->block->cstride)=1.0;
        *(v->block->I->array+v->offset*v->block->cstride)=0.0;
        chouseProd_f(v,cmsv_f(U,Us,i,i),svd->w);
        *(v->block->R->array+v->offset*v->block->cstride)=t.r;
        *(v->block->I->array+v->offset*v->block->cstride)=t.i;
    }
    for(i=n-2; i>0; i--){
        ccol_sv_f(cmsv_f(B,Bs,i,i),v,0);
        t.r=*(v->block->R->array+v->offset*v->block->cstride);
        t.i=*(v->block->I->array+v->offset*v->block->cstride);
        *(v->block->R->array+v->offset*v->block->cstride)=1.0;
        *(v->block->I->array+v->offset*v->block->cstride)=0.0;
        chouseProd_f(v,cmsv_f(U,Us,i,i),svd->w);
        *(v->block->R->array+v->offset*v->block->cstride)=t.r;
        *(v->block->I->array+v->offset*v->block->cstride)=t.i;
    }
    ccol_sv_f(cmsv_f(B,Bs,0,0),v,0);
    t.r=*(v->block->R->array+v->offset*v->block->cstride);
    t.i=*(v->block->I->array+v->offset*v->block->cstride);
    *(v->block->R->array+v->offset*v->block->cstride)=1.0;
    *(v->block->I->array+v->offset*v->block->cstride)=0.0;
    chouseProd_f(v,cmsv_f(U,Us,0,0),svd->w);
    *(v->block->R->array+v->offset*v->block->cstride)=t.r;
    *(v->block->I->array+v->offset*v->block->cstride)=t.i;
}
static void cbidiag_f(csvdObj_f *svd)
{
    vsip_cmview_f *B = svd->B;
    vsip_cmview_f *Bs = &svd->Bs;
    vsip_length m = B->col_length;
    vsip_length n = B->row_length;
    vsip_cvview_f *x=ccol_sv_f(B,&svd->bs,0);
    vsip_cvview_f *v=svd->t;
    vsip_cvview_f *vs = &svd->ts;
    vsip_index i,j;
    vsip_cscalar_f z;
    vsip_scalar_f re,im;
    v->length = x->length;
    cvcopy_f(x,v);
    for(i=0; i<n-1; i++){
        v->length=m-i;
        ccol_sv_f(cmsv_f(B,Bs,i,i),x,0);
        cvcopy_f(x,v);/*0*/
        chouseVector_f(v);
        re=*(v->block->R->array+v->offset*v->block->cstride);
        im=*(v->block->I->array+v->offset*v->block->cstride);
        z.i=(re*re+im*im);z.r=re/z.i;z.i=-im/z.i;
        csvmul_f(z,v);
        chouseProd_f(v,Bs,svd->w);
        cvsv_f(v,vs,1);cvsv_f(x,x,1);
        cvcopy_f(vs,x);/*1*/
        if(i < n-2){
            j = i+1;
            v->length=n-j;
            crow_sv_f(cmsv_f(B,Bs,i,j),x,0);
            cvcopy_f(x,v);/*2*/
            chouseVector_f(v);
            {
                vsip_length _n = v->length;
                vsip_scalar_f *vpi = v->block->I->array + v->block->cstride * v->offset;
                vsip_stride vst = v->block->cstride * v->stride;
                while(_n-- > 0){
                    *vpi = - *vpi; vpi += vst;
                }
                
            }/*v.conj*/
            re=*(v->block->R->array+v->offset*v->block->cstride);
            im=*(v->block->I->array+v->offset*v->block->cstride);
            z.i=(re*re+im*im);z.r=re/z.i;z.i=-im/z.i;
            csvmul_f(z,v);
            cprodHouse_f(Bs,v,svd->w);
            cvsv_f(v,vs,1);cvsv_f(x,x,1);
            cvcopy_f(vs,x);/*3*/
        }
    }
    if(m > n){
        i=n-1;
        v->length=m-i;
        ccol_sv_f(cmsv_f(B,Bs,i,i),x,0);
        cvcopy_f(x,v);/*4*/
        chouseVector_f(v);
        re=*(v->block->R->array+v->offset*v->block->cstride);
        im=*(v->block->I->array+v->offset*v->block->cstride);
        z.i=(re*re+im*im);z.r=re/z.i;z.i=-im/z.i;
        csvmul_f(z,v);
        chouseProd_f(v,Bs,svd->w);
        cvsv_f(v,vs,1);cvsv_f(x,x,1);
        cvcopy_f(vs,x);/*5*/
    }
}
static void csvdBidiag_f(csvdObj_f *svd)
{
    vsip_vview_f*rbs;
    svd->eps0=cmnormFro_f(svd->B)/(vsip_scalar_f)(svd->B->row_length) * 1E-10;
    cbidiag_f(svd);
    cUmatExtract_f(svd);
    cVHmatExtract_f(svd);
    cbiDiagPhaseToZero_f(svd);
    rbs=&svd->rbs;
    vreal_sv_f(cdiag_sv_f(svd->B, &svd->bs, 0),&svd->rbs);
    vcopy_f(rbs,svd->d);
    vreal_sv_f(cdiag_sv_f(svd->B, &svd->bs, 1),&svd->rbs);
    vcopy_f(rbs,svd->f);
}

static void cgtProd_f(vsip_index i, vsip_index j,vsip_scalar_f c, vsip_scalar_f s,csvdObj_f *svd)
{
    vsip_cmview_f* R=&svd->Rs;
    vsip_cvview_f *a1= crow_sv_f(R,&svd->rs_one,i);
    vsip_cvview_f *a2= crow_sv_f(R,&svd->rs_two,j);
    vsip_index k;
    vsip_offset o = a1->block->R->rstride * a1->offset;
    vsip_stride std=a1->stride * a1->block->R->rstride;
    vsip_length n = a1->length;
    register vsip_scalar_f *a1r= a1->block->R->array;
    register vsip_scalar_f *a1i= a1->block->I->array;
    register vsip_scalar_f *a2r= a1r;
    register vsip_scalar_f *a2i= a1i;
    register vsip_scalar_f b1r,b1i,b2r,b2i;
    a1r+=o; a1i+=o;
    o=a2->block->R->rstride * a2->offset;
    a2r+=o; a2i+=o;
    for(k=0; k<n; k++){
        b1r = *a1r, b1i=*a1i, b2r=*a2r, b2i=*a2i;
        *a1r =c * b1r + s * b2r;
        *a1i =c * b1i + s * b2i;
        *a2r =c * b2r - s * b1r;
        *a2i =c * b2i - s * b1i;
        a1r+=std;a1i+=std;a2r+=std;a2i+=std;
    }
}
static void cprodG_f(csvdObj_f *svd,vsip_index i, vsip_index j,vsip_scalar_f c, vsip_scalar_f s)
{
    vsip_cmview_f* L=&svd->Ls;
    vsip_cvview_f *a1= ccol_sv_f(L,&svd->ls_one,i);
    vsip_cvview_f *a2= ccol_sv_f(L,&svd->ls_two,j);
    vsip_index k;
    vsip_offset o = a1->block->R->rstride * a1->offset;
    register vsip_stride std=a1->stride * a1->block->R->rstride;
    vsip_length n = a1->length;
    register vsip_scalar_f *a1r= a1->block->R->array;
    register vsip_scalar_f *a1i= a1->block->I->array;
    register vsip_scalar_f *a2r= a1r;
    register vsip_scalar_f *a2i= a1i;
    register vsip_scalar_f b1r,b1i,b2r,b2i;
    a1r+=o; a1i+=o;
    o=a2->block->R->rstride * a2->offset;
    a2r+=o; a2i+=o;
    for(k=0; k<n; k++){
        b1r = *a1r, b1i=*a1i, b2r=*a2r, b2i=*a2i;
        *a1r =c * b1r + s * b2r;
        *a1i =c * b1i + s * b2i;
        *a2r =c * b2r - s * b1r;
        *a2i =c * b2i - s * b1i;
        a1r+=std;a1i+=std;a2r+=std;a2i+=std;
    }
}
static void czeroCol_f(csvdObj_f *svd)
{
    vsip_vview_f *d = &svd->ds;
    vsip_vview_f *f = &svd->fs;
    vsip_length n = f->length;
    givensObj_f g;
    vsip_scalar_f xd,xf,t;
    vsip_index i,j,k;
    vsip_scalar_f *dptr=d->block->array + d->offset*d->block->rstride;
    vsip_scalar_f *fptr=f->block->array + f->offset*f->block->rstride;
    vsip_stride dstd=d->stride*d->block->rstride,fstd=f->stride*f->block->rstride;
    if (n == 1){
        xd=*dptr;
        xf=*fptr;
        g=givensCoef_f(xd,xf);
        *dptr=g.r;
        *fptr=0.0;
        cgtProd_f(0,1,g.c,g.s,svd);
    }else if (n == 2){
        xd=*(dptr+dstd);
        xf=*(fptr+fstd);
        g=givensCoef_f(xd,xf);
        *(dptr+dstd)=g.r;
        *(fptr+fstd)=0.0;
        xf=*fptr;
        t= -xf * g.s; xf *= g.c;
        *fptr=xf;
        cgtProd_f(1,2,g.c,g.s,svd);
        xd=*dptr;
        g=givensCoef_f(xd,t);
        *dptr=g.r;
        cgtProd_f(0,2,g.c,g.s,svd);
    }else{
        i=n-1; j=i-1; k=i;
        xd=*(dptr+i*dstd);
        xf=*(fptr+i*fstd);
        g=givensCoef_f(xd,xf);
        xf=*(fptr+j*fstd);
        *(fptr+i*fstd)=0.0;
        *(dptr+i*dstd)=g.r;
        t=-xf*g.s; xf*=g.c;
        *(fptr+j*fstd)=xf;
        cgtProd_f(i,k+1,g.c,g.s,svd);
        while (i > 1){
            i = j; j = i-1;
            xd=*(dptr+i*dstd);
            g=givensCoef_f(xd,t);
            *(dptr+i*dstd)=g.r;
            xf=*(fptr+j*fstd);
            t= -xf * g.s; xf *= g.c;
            xf=*(fptr+j*fstd);
            cgtProd_f(i,k+1,g.c,g.s,svd);
        }
        xd=*dptr;
        g=givensCoef_f(xd,t);
        *dptr=g.r;
        cgtProd_f(0,k+1,g.c,g.s,svd);
    }
}

static void czeroRow_f(csvdObj_f *svd)
{
    vsip_vview_f *d = &svd->ds;
    vsip_vview_f *f = &svd->fs;
    vsip_length n = d->length;
    givensObj_f g;
    vsip_scalar_f xd,xf,t;
    vsip_index i;
    vsip_scalar_f *dptr=d->block->array+d->block->rstride*d->offset;
    vsip_scalar_f *fptr=f->block->array+f->block->rstride*f->offset;
    vsip_stride dstrd=d->stride*d->block->rstride;
    vsip_stride fstrd=f->stride*f->block->rstride;
    xd=*dptr;
    xf=*fptr;
    g=givensCoef_f(xd,xf);
    if (n == 1){
        *dptr=g.r;
        *fptr=0.0;
        cprodG_f(svd,1,0,g.c,g.s);
    }else{
        *dptr=g.r;
        *fptr=0.0;
        xf=*(fptr+fstrd);
        t= -xf * g.s; xf *= g.c;
        *(fptr+fstrd)=xf;
        cprodG_f(svd,1,0,g.c,g.s);
        for(i=1; i<n-1; i++){
            xd=*(dptr+i*dstrd);
            g=givensCoef_f(xd,t);
            cprodG_f(svd,i+1,0,g.c,g.s);
            *(dptr+i*dstrd)=g.r;
            xf=*(fptr+(i+1)*fstrd);
            t=-xf * g.s; xf *= g.c;
            *(fptr+(i+1)*fstrd)=xf;
        }
        xd=*(dptr+(n-1)*dstrd);
        g=givensCoef_f(xd,t);
        *(dptr+(n-1)*dstrd)=g.r;
        cprodG_f(svd,n,0,g.c,g.s);
    }
}
static void csvdStep_f(csvdObj_f *svd)
{
    vsip_vview_f *d = &svd->ds;
    vsip_vview_f *f = &svd->fs;
    givensObj_f g;
    vsip_length n = d->length;
    vsip_scalar_f mu=0.0, x1=0.0, x2=0.0;
    vsip_scalar_f t=0.0;
    vsip_index i,j,k;
    vsip_scalar_f d2=0.0,f1=0.0,d3=0.0,f2=0.0;
    vsip_scalar_f *fptr,*dptr,*tdptr,*tfptr;
    vsip_stride fstd=f->stride*f->block->rstride, dstd=d->stride*d->block->rstride;
    dptr=d->block->array+d->offset*d->block->rstride;
    fptr=f->block->array+f->offset*f->block->rstride;
    if(n >= 3){
        d2=*(dptr+dstd*(n-2));f1= *(fptr+fstd*(n-3));d3 = *(dptr+dstd*(n-1));f2= *(fptr+fstd*(n-2));
    } else if(n == 2){
        d2=*dptr; d3=*(dptr+dstd); f1=0; f2=*fptr;
    } else {
        printf("should not be here (see svdStep_f");
        exit(-1);
    }
    mu = svdMu_f(d2,f1,d3,&f2);
    if(f2 == 0.0) *(fptr+fstd*(n-2)) = 0.0;
    x1=*dptr;
    x2 = x1 * *fptr;
    x1 *= x1; x1 -= mu;
    g=givensCoef_f(x1,x2);
    x1=*dptr;x2=*fptr;
    *fptr=g.c * x2 - g.s * x1;
    *dptr=x1 * g.c + x2 * g.s;
    tdptr=dptr+dstd;
    t=*tdptr; *tdptr=t*g.c;
    t*=g.s;
    cgtProd_f(0,1,g.c,g.s,svd);
    for(i=0; i<n-2; i++){
        j=i+1; k=i+2;
        tdptr=dptr+i*dstd;
        tfptr=fptr+i*fstd;
        g = givensCoef_f(*tdptr,t);
        *tdptr=g.r;
        x1 = *(tdptr+dstd)*g.c;
        x2=*tfptr*g.s;
        t= x1 - x2;
        x1=*(fptr+i*fstd) * g.c;
        x2=*(dptr+j*dstd) * g.s ;
        *(fptr+i*fstd)= x1+x2;
        *(dptr+j*dstd) = t;
        x1=*(fptr+j*fstd);
        t=g.s * x1;
        *(fptr+j*fstd) = x1*g.c;
        cprodG_f(svd,i, j, g.c, g.s);
        g=givensCoef_f(*(fptr+i*fstd),t);
        *(fptr+i*fstd)=g.r;
        x1=*(dptr+j*dstd); x2=*(fptr+j*fstd);
        *(dptr+j*dstd)=g.c * x1 + g.s * x2;
        *(fptr+j*fstd)=g.c * x2 - g.s * x1;
        x1=*(dptr+k*dstd);
        t=g.s * x1;
        *(dptr+k*dstd)=x1*g.c;
        cgtProd_f(j,k, g.c, g.s,svd);
    }
    i=n-2; j=n-1;
    g = givensCoef_f(*(dptr+i*dstd),t);
    *(dptr+i*dstd)=g.r;
    x1=*(dptr+j*dstd)*g.c;
    x2=*(fptr+i*fstd)*g.s;
    t=x1 - x2;
    x1 = *(fptr+i*fstd) * g.c; x2=*(dptr+j*dstd) * g.s;
    *(fptr+i*fstd)=x1+x2;
    *(dptr+j*dstd)=t;
    cprodG_f(svd,i, j, g.c, g.s);
}
static void csvdIteration_f(csvdObj_f *svd)
{
    vsip_cmview_f *L0=svd->L;
    vsip_vview_f *d0 = svd->d;
    vsip_vview_f *f0 = svd->f;
    vsip_cmview_f *R0 = svd->R;
    vsip_scalar_f eps0 = svd->eps0;
    vsip_vview_f *d=&svd->ds;
    vsip_vview_f *f=&svd->fs;
    vsip_cmview_f *L=&svd->Ls;
    vsip_cmview_f *R=&svd->Rs;
    vsip_length n;
    svdCorner cnr;
    vsip_index k;
    vsip_length cntr=0;
    vsip_length maxcntr=5*d0->length;
    *d=*d0;*f=*f0;
    while (cntr++ < maxcntr){
        cphaseCheck_f(svd);
        cnr=svdCorners_f(f0);
        if (cnr.j == 0)
            break;
        ivsv_f(d0,d,cnr.i,cnr.j);
        ivsv_f(f0,f,cnr.i,cnr.j-1);
        cimsv_f(L0,L,0,0,cnr.i,cnr.j);
        cimsv_f(R0,R,cnr.i,cnr.j,0,0);
        n=f->length;
        k=zeroFind_f(d,eps0);
        if (k > 0){
            k=k-1;
            if(VI_VGET_F(d,n) == 0.0){
                czeroCol_f(svd);
                printf("zeroCol\n");
            }else{
                printf("zeroRow\n");
                cimsv_f(L,L,0,0,k,0);
                d->length-=(k+1);
                d->offset += (k+1) * d->stride*d->block->rstride;
                f->length -= k;
                f->offset += k * f->stride*f->block->rstride;
                czeroRow_f(svd);
            }
        }else{
            csvdStep_f(svd);
        }
    }
#ifdef DEBUG
    printf("maxcounter %lu, count %lu\n",maxcntr,cntr);
#endif
}

static void svdSort_f(svdObj_f *svd)
{
    vsip_vview_f *d = svd->d;
    vsip_length n=d->length;
    vsip_vview_vi* indx_L = svd->indx_L;
    vsip_vview_vi* indx_R = svd->indx_R;
    vsip_mview_f *L0 = svd->L; vsip_mview_f *L=&svd->Ls;
    vsip_mview_f *R0 = svd->R;
    vsip_vsortip_f(d,VSIP_SORT_BYVALUE,VSIP_SORT_DESCENDING,VSIP_TRUE,indx_L);
    vcopy_vi(indx_L,indx_R);
    imsv_f( L0, L, 0,0, 0, n);
    mpermute_onceCol_f(L,indx_L);
    mpermute_onceRow_f(R0,indx_R);
}
static void csvdSort_f(csvdObj_f *svd)
{
    vsip_cmview_f* L0 = svd->L;
    vsip_vview_f* d = svd->d;
    vsip_cmview_f* R0 = svd->R;
    vsip_length n=d->length;
    vsip_vview_vi* indx_L = svd->indx_L;
    vsip_vview_vi* indx_R = svd->indx_R;
    vsip_cmview_f *L=&svd->Ls;
    vsip_vsortip_f(d,VSIP_SORT_BYVALUE,VSIP_SORT_DESCENDING,VSIP_TRUE,indx_L);
    vcopy_vi(indx_L,indx_R);
    cimsv_f( L0, L, 0,0, 0, n);
    cmpermute_onceCol_f(L,indx_L);
    cmpermute_onceRow_f(R0,indx_R);
}

static void svdFinalize_f(svdObj_f *s)
{
    if(s) {
        vsip_valldestroy_f(s->t);
        vsip_valldestroy_f(s->w);
        vsip_malldestroy_f(s->B);
        vsip_malldestroy_f(s->R);
        vsip_malldestroy_f(s->L);
        vsip_valldestroy_vi(s->indx_L);
        vsip_valldestroy_vi(s->indx_R);
        vsip_valldestroy_f(s->d);
        vsip_valldestroy_f(s->f);
        free(s);
    }
    s=NULL;
}
static svdObj_f* svdInit_f(vsip_length m, vsip_length n)
{
    svdObj_f *s=malloc(sizeof(svdObj_f));
    if(m < n){
        printf("Column length must not be less than row length");
        return NULL;
    }
    if(!s) {
        printf("\nfailed to allocate svd object\n");
        return NULL;
    }
    s->init=0;
    if(!(s->t = vsip_vcreate_f(m,VSIP_MEM_NONE))) s->init++; else s->ts = *s->t;
    if(!(s->w = vsip_vcreate_f(m,VSIP_MEM_NONE))) s->init++;
    if(!(s->B=vsip_mcreate_f(m,n,VSIP_ROW,VSIP_MEM_NONE))) s->init++;
    if(!(s->L=vsip_mcreate_f(m,m,VSIP_ROW,VSIP_MEM_NONE)))s->init++;
    if(!(s->R=vsip_mcreate_f(n,n,VSIP_ROW,VSIP_MEM_NONE)))s->init++;
    if(!(s->indx_L=vsip_vcreate_vi(n,VSIP_MEM_NONE))) s->init++;
    if(!(s->indx_R=vsip_vcreate_vi(n,VSIP_MEM_NONE))) s->init++;
    if(!(s->d = vsip_vcreate_f(n,VSIP_MEM_NONE))) s->init++;
    if(!(s->f = vsip_vcreate_f(n-1,VSIP_MEM_NONE)))s->init++;
    if(s->init){
        svdFinalize_f(s);
        return NULL;
    }
    meye_f(s->L);
    meye_f(s->R);
    return s;
}
static void svd_f(svdObj_f *s)
{
    svdBidiag_f(s);
    svdIteration_f(s);
    svdSort_f(s);
}
static void csvdFinalize_f(csvdObj_f *s)
{
    if(s) {
        vsip_cvalldestroy_f((s)->t);
        vsip_cvalldestroy_f((s)->w);
        vsip_cmalldestroy_f((s)->B);
        vsip_cmalldestroy_f((s)->R);
        vsip_cmalldestroy_f((s)->L);
        vsip_valldestroy_vi((s)->indx_L);
        vsip_valldestroy_vi((s)->indx_R);
        vsip_valldestroy_f((s)->d);
        vsip_valldestroy_f((s)->f);
        free(s);
    }
    s=NULL;
}
static csvdObj_f* csvdInit_f(vsip_length m, vsip_length n)
{
    csvdObj_f *s=malloc(sizeof(csvdObj_f));
    if(m < n){
        printf("Column length must not be less than row length");
        return NULL;
    }
    if(!s) {
        printf("\nfailed to allocate svd object\n");
        return NULL;
    }
    s->init=0;
    if(!(s->t = vsip_cvcreate_f(m,VSIP_MEM_NONE)))s->init++;else s->ts = *s->t;
    if(!(s->w = vsip_cvcreate_f(m,VSIP_MEM_NONE)))s->init++;
    if(!(s->B=vsip_cmcreate_f(m,n,VSIP_ROW,VSIP_MEM_NONE)))s->init++;
    if(!(s->L=vsip_cmcreate_f(m,m,VSIP_ROW,VSIP_MEM_NONE)))s->init++;
    if(!(s->R=vsip_cmcreate_f(n,n,VSIP_ROW,VSIP_MEM_NONE)))s->init++;
    if(!(s->indx_L=vsip_vcreate_vi(n,VSIP_MEM_NONE))) s->init++;
    if(!(s->indx_R=vsip_vcreate_vi(n,VSIP_MEM_NONE))) s->init++;
    if(!(s->d = vsip_vcreate_f(n,VSIP_MEM_NONE)))s->init++;
    if(!(s->f = vsip_vcreate_f(n-1,VSIP_MEM_NONE)))s->init++;
    if(s->init){
        csvdFinalize_f(s);
        return NULL;
    }
    cmeye_f(s->L);
    cmeye_f(s->R);
    return s;
}
static void csvd_f(csvdObj_f *s)
{
    csvdBidiag_f(s);
    csvdIteration_f(s);
    csvdSort_f(s);
}

int
vsip_svd_destroy_f(vsip_sv_f* s)
{
    if(s){
        svdObj_f* svd=(svdObj_f*)s->svd;
        svdFinalize_f(svd);
        free((void*)s);
        s=NULL;
    }
    return 0;
}
vsip_sv_f *
vsip_svd_create_f(vsip_length M, vsip_length N, vsip_svd_uv Usave, vsip_svd_uv Vsave)
{
    vsip_sv_f *s = (vsip_sv_f*) malloc(sizeof(vsip_sv_f));
    if(s){
        if(M < N){
            s->svd = (void*) svdInit_f(N,M);
        }else{
            s->svd = (void*) svdInit_f(M,N);
        }
        if(s->svd){
            s->attr.Usave=Usave;
            s->attr.Vsave=Vsave;
            s->attr.m=M;
            s->attr.n=N;
            s->transpose = M < N ? 1:0;
        } else {
            vsip_svd_destroy_f(s);
        }
    }
    return s;
}
int
vsip_svd_f(vsip_sv_f *svd, const vsip_mview_f *A, vsip_vview_f *sv)
{
    svdObj_f *s = (svdObj_f*) svd->svd;
    if(svd->transpose){
        mtrans_f(A,s->B)
    } else {
        mcopy_f(A,s->B)
    }
    svd_f(s);
    vcopy_f(s->d,sv)
    return 0;
}
void
vsip_svd_getattr_f(const vsip_sv_f *svd, vsip_sv_attr_f *attrib)
{
    attrib->Usave = svd->attr.Usave;
    attrib->Vsave = svd->attr.Vsave;
    attrib->m = svd->attr.m;
    attrib->n = svd->attr.n;
}
int
vsip_svdprodu_f(const vsip_sv_f *svd, vsip_mat_op OpU, vsip_mat_side ApU, const vsip_mview_f *C)
{
    if((OpU != VSIP_MAT_NTRANS) || (OpU != VSIP_MAT_TRANS)) return 1;
    if( (ApU != VSIP_MAT_LSIDE) || (ApU != VSIP_MAT_RSIDE)) return 1;
    if( svd->attr.Usave == VSIP_SVD_UVPART){
        if (ApU == VSIP_MAT_LSIDE){
            if(OpU == VSIP_MAT_NTRANS){
            } else {
            }
        } else { /* must be MAT_RSIDE */
            if(OpU == VSIP_MAT_NTRANS){
            } else {
            }
        }
    } else if(svd->attr.Usave == VSIP_SVD_UVFULL){
        if (ApU == VSIP_MAT_LSIDE){
            if(OpU == VSIP_MAT_NTRANS){
            } else {
            }
        } else { /* must be MAT_RSIDE */
            if(OpU == VSIP_MAT_NTRANS){
            } else {
            }
        }
    } else {
        return 1;
    }
    return 0;
}


int
vsip_svdprodv_f(const vsip_sv_f *svd, vsip_mat_op OpV, vsip_mat_side ApV,const vsip_mview_f *C)
{
    return 0;
}

int
vsip_svdmatu_f(const vsip_sv_f *svd, vsip_scalar_vi low, vsip_scalar_vi high, const vsip_mview_f *C)
{
    svdObj_f *s = (svdObj_f*) svd->svd;
    if(svd->transpose){
        vsip_mview_f *L = s->R;
        vsip_mview_f *Ls = &s->Rs;
        Ls->offset=L->offset + L->col_stride * low;
        Ls->row_stride=L->col_stride; Ls->col_stride=L->row_stride;
        Ls->row_length = (high-low) + 1; Ls->col_length=L->row_length;
        mcopy_f(Ls,C);
    } else {
        vsip_mview_f *L = s->L;
        vsip_mview_f *Ls = &s->Ls;
        Ls->offset = L->offset + L->row_stride * low;
        Ls->row_length = (high-low) + 1;
        Ls->col_length = L->col_length;
        Ls->row_stride = L->row_stride;
        Ls->col_stride = L->col_stride;
        mcopy_f(Ls,C);
    }
    return 0;
}

int
vsip_svdmatv_f(const vsip_sv_f *svd, vsip_scalar_vi low, vsip_scalar_vi high, const vsip_mview_f *C)
{
    svdObj_f *s = (svdObj_f*) svd->svd;
    if(svd->transpose){
        vsip_mview_f *R  = s->L;
        vsip_mview_f *Rs = &s->Ls;
        *Rs=*R;
        Rs->offset += Rs->row_stride * low;
        Rs->row_length = (high-low) + 1;
        mcopy_f(Rs,C);
    } else {
        vsip_mview_f *R  = s->R;
        vsip_mview_f *Rs = &s->Rs;
        Rs->offset=R->offset;
        Rs->row_stride=R->col_stride;
        Rs->col_stride=R->row_stride;
        Rs->row_length=R->col_length;
        Rs->col_length=R->row_length;
        Rs->offset += Rs->row_stride * low;
        Rs->row_length = (high-low) + 1;
        mcopy_f(Rs,C);
    }
    return 0;
}

/* Complex */
int
vsip_csvd_destroy_f(vsip_csv_f* s)
{
    if(s){
        csvdObj_f* svd=(csvdObj_f*)s->svd;
        csvdFinalize_f(svd);
        s->svd=NULL;
        free((void*)s);
        s=NULL;
    }
    return 0;
}

vsip_csv_f *
vsip_csvd_create_f(vsip_length M, vsip_length N, vsip_svd_uv Usave, vsip_svd_uv Vsave)
{
    vsip_csv_f *s = (vsip_csv_f*) malloc(sizeof(vsip_csv_f));
    if(s){
        if(M < N){
            s->svd = (void*) csvdInit_f(N,M);
        }else{
            s->svd = (void*)csvdInit_f(M,N);
        }
        if(s->svd){
            s->attr.Usave=Usave;
            s->attr.Vsave=Vsave;
            s->attr.m=M;
            s->attr.n=N;
            s->transpose = M < N ? 1:0;
        } else {
            vsip_svd_destroy_f(s);
            s=NULL;
        }
    }
    return s;
}

int
vsip_csvd_f(vsip_csv_f *svd, const vsip_cmview_f *A, vsip_vview_f *sv)
{
    csvdObj_f *s = (csvdObj_f*) svd->svd;
    if(svd->transpose){
        cmherm_f(A,s->B);
    } else {
        cmcopy_f(A,s->B);
    }
    csvd_f(s);
    vcopy_f(s->d,sv);
    return 0;
}

void
vsip_csvd_getattr_f(const vsip_csv_f *svd, vsip_csv_attr_f *attrib)
{
    attrib->Usave = svd->attr.Usave;
    attrib->Vsave = svd->attr.Vsave;
    attrib->m = svd->attr.m;
    attrib->n = svd->attr.n;
}

int
vsip_csvdprodu_f(const vsip_sv_f *svd, vsip_mat_op OpU, vsip_mat_side ApU,
                  const vsip_mview_f *C)
{
    return 0;
}

int
vsip_csvdprodv_f(const vsip_csv_f *svd, vsip_mat_op OpV, vsip_mat_side ApV,const vsip_cmview_f *C)
{
    return 0;
}

int
vsip_csvdmatu_f(const vsip_csv_f *svd, vsip_scalar_vi low, vsip_scalar_vi high, const vsip_cmview_f *C)
{
    csvdObj_f *s = (csvdObj_f*) svd->svd;
    if(svd->transpose){
        vsip_cmview_f *L = s->R;
        vsip_cmview_f *Ls = &s->Rs;
        Ls->offset=L->offset + L->col_stride * low;
        Ls->row_stride=L->col_stride; Ls->col_stride=L->row_stride;
        Ls->row_length = (high-low) + 1; Ls->col_length=L->row_length;
        cmconj_f(Ls,C);
    } else {
        vsip_cmview_f *L = s->L;
        vsip_cmview_f *Ls = &s->Ls;
        Ls->offset = L->offset + L->row_stride * low;
        Ls->row_length = (high-low) + 1;
        Ls->row_stride = L->row_stride;
        Ls->col_length = L->col_length;
        Ls->col_stride = L->col_stride;
        cmcopy_f(Ls,C);
    }
    return 0;
}

int
vsip_csvdmatv_f(const vsip_csv_f *svd, vsip_scalar_vi low, vsip_scalar_vi high, const vsip_cmview_f *C)
{
    csvdObj_f *s = (csvdObj_f*) svd->svd;
    if(svd->transpose){
        vsip_cmview_f *R  = s->L;
        vsip_cmview_f *Rs = &s->Ls;
        *Rs=*R;
        Rs->offset += Rs->row_stride * low;
        Rs->row_length = (high-low) + 1;
        cmcopy_f(Rs,C);
    } else {
        vsip_cmview_f *R  = s->R;
        vsip_cmview_f *Rs = &s->Rs;
        Rs->offset=R->offset;
        Rs->row_stride=R->col_stride;
        Rs->col_stride=R->row_stride;
        Rs->row_length=R->col_length;
        Rs->col_length=R->row_length;
        Rs->offset += Rs->row_stride * low;
        Rs->row_length = (high-low) + 1;
        cmconj_f(Rs,C);
    }
    return 0;
}
