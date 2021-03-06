print('TEST vsiputils create and destroy routines')
print('this test just does a create and destroy for each supported VSIP type')
print('No \'additional\' output means the test passes')
import vsiputils as vsip
#block create/destroy
a=vsip.create('block_f',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('block_d',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('cblock_f',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('cblock_d',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('block_i',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('block_si',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('block_uc',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('block_vi',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('block_mi',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('block_bl',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
#vector view create/destroy
a=vsip.create('vview_d',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('vview_f',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('vview_d',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('cvview_f',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('cvview_d',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('vview_i',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('vview_si',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('vview_uc',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('vview_vi',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('vview_mi',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('vview_bl',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
# rand create/destroy
a=vsip.create('randstate',(3,1,1,vsip.VSIP_PRNG))
vsip.destroy(a)
# window create destroy
a=vsip.create('blackman_d',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('blackman_f',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('hanning_d',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('hanning_f',(100,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('kaiser_d',(100,0.5,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('kaiser_f',(100,0.35,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('cheby_d',(100,50,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
a=vsip.create('cheby_f',(100,35,vsip.VSIP_MEM_NONE))
vsip.destroy(a)
#lu create destroy
a=vsip.create('lu_d',10)
vsip.destroy(a)
a=vsip.create('lu_f',(10,))
vsip.destroy(a)
a=vsip.create('clu_f',(10))
vsip.destroy(a)
a=vsip.create('clu_d',10)
vsip.destroy(a)
#cholesky solvers create/destroy
a=vsip.create('chol_f',(vsip.VSIP_TR_LOW,10))
vsip.destroy(a)
a=vsip.create('chol_d',(vsip.VSIP_TR_UPP,10))
vsip.destroy(a)
a=vsip.create('cchol_f',(vsip.VSIP_TR_LOW,10))
vsip.destroy(a)
a=vsip.create('cchol_d',(vsip.VSIP_TR_UPP,10))
vsip.destroy(a)
# qr solvers create/destroy
a=vsip.create('qr_f',(10,5,vsip.VSIP_QRD_NOSAVEQ))
vsip.destroy(a)
a=vsip.create('qr_d',(9,4,vsip.VSIP_QRD_SAVEQ))
vsip.destroy(a)
a=vsip.create('cqr_f',(11,7,vsip.VSIP_QRD_NOSAVEQ))
vsip.destroy(a)
a=vsip.create('cqr_d',(8,4,vsip.VSIP_QRD_SAVEQ1))
vsip.destroy(a)
#svd solvers create/destroy
a=vsip.create('sv_f',(10,5,vsip.VSIP_SVD_UVNOS,vsip.VSIP_SVD_UVPART))
vsip.destroy(a)
a=vsip.create('sv_d',(7,9,vsip.VSIP_SVD_UVFULL,vsip.VSIP_SVD_UVNOS))
vsip.destroy(a)
#fir filters create/destroy
b=vsip.create('vview_f',(10,vsip.VSIP_MEM_NONE))
a=vsip.create('fir_f',(b,vsip.VSIP_NONSYM,100,2,vsip.VSIP_STATE_NO_SAVE,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
vsip.destroy(b)
b=vsip.create('vview_d',(11,vsip.VSIP_MEM_NONE))
a=vsip.create('fir_d',(b,vsip.VSIP_SYM_EVEN_LEN_ODD,101,5,vsip.VSIP_STATE_SAVE,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
vsip.destroy(b)
b=vsip.create('cvview_f',(8,vsip.VSIP_MEM_NONE))
a=vsip.create('cfir_f',(b,vsip.VSIP_SYM_EVEN_LEN_EVEN,150,3,vsip.VSIP_STATE_NO_SAVE,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
vsip.destroy(b)
b=vsip.create('cvview_d',(6,vsip.VSIP_MEM_NONE))
a=vsip.create('cfir_d',(b,vsip.VSIP_NONSYM,104,1,vsip.VSIP_STATE_SAVE,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
vsip.destroy(b)
#FFT create/destroy
a=vsip.create('ccfftop_f',(100,1,vsip.VSIP_FFT_FWD,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('ccfftop_d',(101,.5,vsip.VSIP_FFT_FWD,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('ccfftip_f',(102,.1,vsip.VSIP_FFT_INV,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('ccfftip_d',(103,2,vsip.VSIP_FFT_INV,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('rcfftop_f',(100,2,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('rcfftop_d',(100,1,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('crfftop_f',(100,.001,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('crfftop_d',(100,1.0/100.0,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
#FFTM create/destroy
a=vsip.create('ccfftmop_f',(10,9,1.0,vsip.VSIP_FFT_FWD,vsip.VSIP_ROW,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('ccfftmop_d',(10,9,1.0,vsip.VSIP_FFT_INV,vsip.VSIP_COL,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('ccfftmip_f',(10,9,1.0,vsip.VSIP_FFT_INV,vsip.VSIP_COL,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('ccfftmip_d',(10,9,1.0,vsip.VSIP_FFT_FWD,vsip.VSIP_ROW,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('rcfftmop_f',(100,11,1.0,vsip.VSIP_COL,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('rcfftmop_d',(10,100,1.0,vsip.VSIP_ROW,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('crfftmop_f',(11,100,1.0,vsip.VSIP_ROW,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
a=vsip.create('crfftmop_d',(8,64,1.0,vsip.VSIP_COL,0,vsip.VSIP_ALG_TIME))
vsip.destroy(a)
