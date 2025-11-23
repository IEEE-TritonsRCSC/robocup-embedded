# PID Tuning Results
**Date:** November 12, 2025  
**Robot ID:** 1  
**Method:** Automated step response analysis with elevated wheels

---

## Final Tuned Values

| Wheel | Position | Kp | Ki | Kd | Steady-State Error | Notes |
|-------|----------|----|----|----|--------------------|-------|
| 0 | Front Right | 1.125 | 0.050 | 0.0 | ~11-13% | Rise time: 0.8s, no oscillation |
| 1 | Back Right | 1.688 | 0.050 | 0.0 | ~4.1% | Good response, minimal error |
| 2 | Back Left | 3.797 | 0.050 | 0.0 | ~2.3% | Most aggressive, best tracking |
| 3 | Front Left | 1.125 | 0.075 | 0.0 | ~6.6% | Higher Ki for better tracking |

---

## Commands to Set PID Values

```bash
# Wheel 0 (Front Right)
echo "1 pidu 0 1125 50 0" | nc -u 239.42.42.42 10000

# Wheel 1 (Back Right)
echo "1 pidu 1 1688 50 0" | nc -u 239.42.42.42 10000

# Wheel 2 (Back Left)
echo "1 pidu 2 3797 50 0" | nc -u 239.42.42.42 10000

# Wheel 3 (Front Left)
echo "1 pidu 3 1125 75 0" | nc -u 239.42.42.42 10000
```

---

## Performance Summary

### Wheel 0 (Front Right)
- **Starting values:** Kp=0.5, Ki=0.05
- **Tuning iterations:** 2
- **Key improvements:** 
  - Increased Kp from 0.5 → 1.125 (2.25x)
  - Rise time: 0.8s
  - Fast initial response
  - ~11% steady-state error acceptable for this wheel

### Wheel 1 (Back Right)
- **Starting values:** Kp=1.125, Ki=0.05
- **Tuning iterations:** 2
- **Key improvements:**
  - Increased Kp from 1.125 → 1.688 (1.5x)
  - Steady-state error reduced to 4.1%
  - Good settling characteristics

### Wheel 2 (Back Left)
- **Starting values:** Kp=1.125, Ki=0.05
- **Tuning iterations:** 3
- **Key improvements:**
  - Increased Kp from 1.125 → 3.797 (3.4x - most aggressive!)
  - Steady-state error: 6.7% → 4.6% → 2.3%
  - Best tracking of all wheels
  - This wheel appears to have more friction/load

### Wheel 3 (Front Left)
- **Starting values:** Kp=1.125, Ki=0.05
- **Tuning iterations:** 2
- **Key improvements:**
  - Kept Kp=1.125
  - Increased Ki from 0.05 → 0.075 (1.5x)
  - Rise time: 0.1s (fastest!)
  - Steady-state error: 9.5% → 6.6%

---

## Observations

1. **Wheel variation:** Wheels required different gains (Kp from 1.125 to 3.797), suggesting:
   - Manufacturing tolerances in motors
   - Different mechanical loads
   - Possible friction differences

2. **Consistent Ki:** Most wheels work well with Ki=0.05, except wheel 3 which benefits from Ki=0.075

3. **No D term needed:** All wheels stable with Kd=0, suggesting:
   - Low noise system
   - Good velocity feedback from encoders
   - Derivative term not necessary for this application

4. **Fast response:** Rise times of 0.1-0.8s are excellent for this type of robot

---

## Next Steps

1. **Test on ground:** Current tuning done with wheels elevated (no load)
2. **Fine-tune if needed:** May need slight Kp reduction when on ground due to friction
3. **Verify stability:** Test at various speeds and directions
4. **Monitor telemetry:** Use `pid_plotter.py` to visualize real-world performance

---

## Files Generated
- `autotune_wheel0_*.json` - Detailed data for wheel 0
- `autotune_wheel1_*.json` - Detailed data for wheel 1
- `autotune_wheel2_*.json` - Detailed data for wheel 2
- `autotune_wheel3_*.json` - Detailed data for wheel 3
