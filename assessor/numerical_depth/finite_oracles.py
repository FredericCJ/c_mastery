"""Independent exact derivations for the named T17 analysis-only transfer tasks."""
from fractions import Fraction as F
from decimal import Decimal,localcontext
import itertools,json,math
out={}
signal=[F(1000),F(999),F(-1998)];total=sum(signal);condition=sum(map(abs,signal))/abs(total)
assert total==1 and condition==3997
out['units']={'millivolt_sum':str(total),'volt_sum':str(total/1000),'condition_both':str(condition),'atol_mV':'1/100','atol_V':'1/100000','near_zero':'Use absolute tolerance; a relative-only criterion has no useful denominator at exact zero.','normalized_controller':'u=(sum_mV-reference_mV)/fullscale_mV; reject nonpositive fullscale and state admissible sensor interval before normalization.'}
# The generating phenomenon and fitted model are different, even with exact arithmetic.
u=F(3,2);truth=2*u+1+u*u/100;linear=2*u+1;measurement=truth+F(1,1000);rounded=F(float(measurement));prediction=float(2*float(u)+1)
assert F(prediction)-truth==(F(prediction)-linear)+(linear-truth)
out['calibration']={'true_response':str(truth),'linear_model':str(linear),'model_error':str(linear-truth),'measurement_error':str(measurement-truth),'binary64_input_error':str(rounded-measurement),'algorithm_error_for_exact_linear_input':str(F(prediction)-linear),'nonlinear_change':'The former linear-model acceptance cannot bound the new quadratic response; sampling/calibration model and error budget must be revised.'}
known_bias=F(1,16);linear_measurement=linear+F(1,1000);implemented=float(linear_measurement)+float(known_bias)
out['linear_calibration_budget']={'phenomenon':str(linear),'measurement_bias':'1/1000','known_calibration_bias':str(known_bias),'measurement_representation':str(F(float(linear_measurement))-linear_measurement),'final_addition_roundoff':str(F(implemented)-(F(float(linear_measurement))+known_bias)),'total_error':str(F(implemented)-linear)}
exact=sum(map(F,[float(2**53),1.,float(-2**53)]));assert exact==1
out['reference_inputs']={'integer_sum':'1','absolute_sum':'18014398509481985','decimal_three_tenths':'3/10','binary_three_tenths':str(sum([F(.1)]*3)),'distinction':'Decimal intended input and exactly represented binary64 input answer different questions.'}
for q in [7,11]:
 d=1<<q;minimum=-(d*(d-1));maximum=d*d;assert minimum>=-(2**31) and 32*maximum<2**31
 out['Q'+str(q)]={'product_min':minimum,'product_max':maximum,'mac32_min':32*minimum,'mac32_max':32*maximum,'quantization_error_in_unclipped_interior':str(F(1,2*d)),'physical_gain':'For fullscale S and physical gain G, output error includes |G|S/(2*2^q); clipping adds unbounded-by-half-LSB error outside the admitted physical range.'}
out['rounding_point']={'products':['1*64','1*64'],'round_each_Q7':2,'round_once_Q7':1,'rule':'Magnitude quotient/remainder, ties away from zero, then saturation; negative right shift is not the rounding rule.'}
# A fixed-point integer in quantum2^-1074 exactly accumulates finite binary64 inputs.
def accumulate(xs):
 a=0
 for x in xs:
  p,q=float(x).as_integer_ratio();assert q&(q-1)==0;a+=p*(2**1074//q)
 return F(a,2**1074)
values=[1e100,-1e100,1.,2**-1000]
assert len({accumulate(p) for p in itertools.permutations(values)})==1
out['superaccumulator_design']={'input_count_limit':4096,'quantum':'2^-1074','full_binary64_unsigned_magnitude_bits':2110,'signed_twos_complement_bits':2111,'storage_64_bit_limbs':33,'mechanism':'Decode sign/exponent/significand, shift integer significand into common quantum, add with carries to fixed limbs, merge limbs exactly, round once nearest-even with guard/sticky bits. No per-worker floating rounding. Check finite input and count bound before accumulation.','executed_design_model_permutations':24,'limits':'Python exact-integer design model; not a claimed C limb implementation. Exact input decomposition and bounded limb/carry/rounding design are the original requested transfer artifact.'}
out['quadratic_interpolation']={'uniform_h':'1/2','analytic_max_error':'h^2/4=1/16','max_locations':['1/4','3/4'],'nonuniform_h_max':'7/8','nonuniform_max_error':'49/256','operation_comparison':'Two stored endpoint ordinates and two weighted products plus addition/division per segment; x*x polynomial needs one product and no table for this particular known quadratic. Binary search costs O(log knots); validation costs O(knots).'}
# A continuous triangular feature vanishes at all101 grid samples yet peaks between them.
def bump(x):return max(F(),1-abs(x-F(1,200))/F(1,1000))
assert all(bump(F(i,100))==0 for i in range(101)) and bump(F(1,200))==1
out['missed_feature']={'grid_max':0,'actual_at_1_over_200':1,'required_revision':'Lipschitz bound or adaptive subdivision with a certified derivative/curvature bound, otherwise unobserved narrow features remain possible.'}
out['filter_poles']={'stable_second_order':'(z-9/10)^2: feedback coefficients9/5,-81/100; roots9/10 despite first coefficient>1','unstable_second_order':'(z-6/5)(z-1/5): feedback7/5,-6/25; pole6/5 grows','nonzero_half_IIR':'y[n]=1+(y[-1]-1)*2^-(n+1) for unit step','quantized_feedback':str(F(127,128)),'scope':'First-order |feedback|<1 applies only to this recurrence; it is not a general multi-pole criterion.'}
eps=F(1,2**30);delta=F(1,2**40);solution=[1-delta/eps,1+delta/eps];assert solution==[F(1023,1024),F(1025,1024)]
out['solver_sensitivity']={'epsilon':str(eps),'rhs_delta':str(delta),'solution':list(map(str,solution)),'exact_residual_at_solution':0,'forward_change_inf':'1/1024','threshold':'Scaled pivot screening is a diagnostic policy, not a condition-number bound.','iterative':'An absolute residual stop, iteration cap and unchanged-iterate stagnation are separate outcomes. A small residual alone cannot certify forward accuracy.'}
out['solver_cost']={'2x2_pivot':[3,3,3,1],'3x3_no_swaps':[11,11,6,0],'order':'adds,multiplies,divisions,row_swaps','memory':'2x2 padded row stride3 reads4 logical A and2 b scalars, writes2 solution scalars; row swaps move n+1 scratch values.3x3 padded stride4 reads9 A and3 b, writes3; scratch matrix3x4, scales3 and solution3 doubles plus counters. Input validation/threshold comparisons excluded from arithmetic counts.'}
with localcontext() as ctx:
 ctx.prec=100;continuous=(-Decimal(1)).exp();errors=[]
 for n in [2,4,8]:
  discrete=F(n-1,n)**n;error=abs(Decimal(discrete.numerator)/Decimal(discrete.denominator)-continuous);errors.append(error)
 out['euler']={'exp_minus_one_100_digits':str(continuous),'errors':list(map(str,errors)),'ratios':[str(errors[i]/errors[i+1]) for i in range(2)],'stability':'For y=-rate*y, multiplier1-rate*h. Decay requires0<rate*h<2; equality2 oscillates without decay.','model_change':'For yprime=1-y and y(0)=1, exact solution stays1; the old pure-decay simulation is wrong even if its recurrence test passes. For a two-compartment conservative model integrate both components and verify their sum, rather than imposing the single-state decay assertion.'}
out['layout_cache_model']={'line_bytes':64,'AoS_assumed_record_bytes':48,'n':128,'x_useful_bytes':1024,'AoS_touched_lines':96,'SoA_x_touched_lines':16,'all_fields':'When x,y and24 label bytes are consumed, useful bytes5120; AoS co-locates each record while SoA traverses separate streams. Alignment/start offsets, padding, actual misses and prefetching require separate observations.','stable_handle':'IDs survive compaction/reorder while indices do not; lookup is O(n) in bounded16-store. Slot/index/generation indirection can give O(1) but adds generation exhaustion and owner lifetime obligations.'}
assert len({(48*i)//64 for i in range(128)})==96
print(json.dumps({'passed':True,'scope':'Original exact analysis/transfer derivations; distinct from executed C and measurements','dossiers':out}))
