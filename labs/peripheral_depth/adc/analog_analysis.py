"""Host-only analysis of declared analog models; never a board observation."""
from decimal import Decimal,localcontext
from fractions import Fraction

def settling(resistance_ohm, capacitance_pf, bits, lsb_fraction, adc_hz):
 if not (0<resistance_ohm<=1000000 and 0<capacitance_pf<=1000 and 1<=bits<=24 and 0<lsb_fraction<=1 and adc_hz>0):raise ValueError('RC premise')
 with localcontext() as ctx:
  ctx.prec=50
  ratio=Decimal(2**bits)/Decimal(str(lsb_fraction))
  seconds=Decimal(resistance_ohm)*Decimal(capacitance_pf)*Decimal('1e-12')*ratio.ln()
  required=int((seconds*Decimal(adc_hz)).to_integral_value(rounding='ROUND_CEILING'))
  choices=[v for v in [3,15,28,56,84,112,144,480] if v>=required]
  return dict(required_cycles=required,selected_cycles=choices[0] if choices else None,seconds=str(seconds),model='Worst full-scale exponential step; specified lumped R,C, no switch/leakage/nonlinearity model.')

def threshold_interval(reference_mv,reference_error_mv,offset_low_mv,offset_high_mv,hysteresis_min_mv,hysteresis_max_mv):
 if reference_error_mv<0 or offset_low_mv>offset_high_mv or not 0<=hysteresis_min_mv<=hysteresis_max_mv:raise ValueError('threshold premise')
 # Conservative unknown placement: do not assume a nominal hysteresis center or
 # orientation from a block name. Both thresholds lie in this broad enclosure.
 lo=reference_mv-reference_error_mv+offset_low_mv-hysteresis_max_mv
 hi=reference_mv+reference_error_mv+offset_high_mv+hysteresis_max_mv
 return dict(enclosure_mv=[lo,hi],hysteresis_width_mv=[hysteresis_min_mv,hysteresis_max_mv],limits='A conservative enclosure, not two exact switching thresholds; measure rising/falling levels separately.')

def timestamp_interval(first_tick,last_tick,tick_ns,jitter_ns,propagation_ns):
 if first_tick<0 or last_tick<first_tick or tick_ns<=0 or jitter_ns<0 or propagation_ns<0:raise ValueError('timestamp premise')
 return [Fraction(first_tick*tick_ns-jitter_ns-propagation_ns),Fraction((last_tick+1)*tick_ns+jitter_ns)]
