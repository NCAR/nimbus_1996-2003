
/* -------------------------------------------------------------------- */

double Esubt(double temperature, double pressure)
{
  double  Tk = temperature + To;
  double  fw, ew;

  if (Tk < 1.0)
    Tk = 1.0;

  ew = pow(10.0, (
        -7.90298 * ((Ts / Tk) - 1.0)
        +5.02808 * log10(Ts / Tk)
        -1.3816e-7 * (pow(10.0, 11.334 * (1.0 - Tk / Ts)) - 1.0)
        +8.1328e-3 * (pow(10.0, -3.49149 * (Ts / Tk - 1.0)) - 1.0))
        +log10(1013.246) );

  /* Arden Buck's pressure enhancement factor.
   */
  fw = 1.0007 + (3.46e-6 * pressure);

  return(ew * fw);

}   /* END ESUBT */
