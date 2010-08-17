/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 Header:       FGTurbine.h
 Author:       David Culp
 Date started: 03/11/2003

 ------------- Copyright (C) 2003  David Culp (davidculp2@comcast.net)----------

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 details.

 You should have received a copy of the GNU Lesser General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA  02111-1307, USA.

 Further information about the GNU Lesser General Public License can also be found on
 the world wide web at http://www.gnu.org.

HISTORY
--------------------------------------------------------------------------------
03/11/2003  DPC  Created, based on FGTurbine
09/22/2003  DPC  Added starting, stopping, new framework
04/29/2004  DPC  Renamed from FGSimTurbine to FGTurbine

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
SENTRY
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifndef FGTURBINE_H
#define FGTURBINE_H

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
INCLUDES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#include "FGEngine.h"

#define ID_TURBINE "$Id: FGTurbine.h,v 1.18 2009/10/24 22:59:30 jberndt Exp $"

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
FORWARD DECLARATIONS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

namespace JSBSim {

class Element;
class FGFunction;

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
CLASS DOCUMENTATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/** This class models a turbine engine.  Based on Jon Berndt's FGTurbine module.
    Here the term "phase" signifies the engine's mode of operation.  At any given
    time the engine is in only one phase.  At simulator startup the engine will be
    placed in the Trim phase in order to provide a simplified thrust value without
    throttle lag.  When trimming is complete the engine will go to the Off phase,
    unless the value FGEngine::Running has been previously set to true, in which
    case the engine will go to the Run phase.  Once an engine is in the Off phase
    the full starting procedure (or airstart) must be used to get it running.
<P>
    - STARTING (on ground):
      -# Set the control FGEngine::Starter to true.  The engine will spin up to
         a maximum of about %25 N2 (%5.2 N1).  This simulates the action of a
         pneumatic starter.
      -# After reaching %15 N2 set the control FGEngine::Cutoff to false. If fuel
         is available the engine will now accelerate to idle.  The starter will
         automatically be set to false after the start cycle.
<P>
    - STARTING (in air):
      -# Increase speed to obtain a minimum of %15 N2.  If this is not possible,
         the starter may be used to assist.
      -# Place the control FGEngine::Cutoff to false.
<P>
    Ignition is assumed to be on anytime the Cutoff control is set to false,
    therefore a seperate ignition system is not modeled.

<h3>Configuration File Format:</h3>
@code
 <turbine_engine name="{string}">
  <milthrust unit="{LBS | N}"> {number} </milthrust>
  <maxthrust unit="{LBS | N}"> {number} </maxthrust>
  <bypassratio> {number} </bypassratio>
  <bleed> {number} </bleed>
  <tsfc> {number} </tsfc>
  <atsfc> {number} </atsfc>
  <idlen1> {number} </idlen1>
  <idlen2> {number} </idlen2>
  <maxn1> {number} </maxn1>
  <maxn2> {number} </maxn2>
  <augmented> {0 | 1} </augmented>
  <augmethod> {0 | 1 | 2} </augmethod>
  <injected> {0 | 1} </injected>
  <injection-time> {number} </injection-time>
 </turbine_engine>
@endcode

<h3>Definition of the turbine engine configuration file parameters:</h3>

<pre>
  milthrust   - Maximum thrust, static, at sea level.
  maxthrust   - Afterburning thrust, static, at sea level.
  bypassratio - Ratio of bypass air flow to core air flow.
  bleed       - Thrust reduction factor due to losses (0.0 to 1.0).
  tsfc        - Thrust-specific fuel consumption at cruise, lbm/hr/lbf
  atsfc       - Afterburning TSFC, lbm/hr/lbf
  idlen1      - Fan rotor rpm (% of max) at idle
  idlen2      - Core rotor rpm (% of max) at idle
  maxn1       - Fan rotor rpm (% of max) at full throttle 
  maxn2       - Core rotor rpm (% of max) at full throttle
  augmented
              0 = afterburner not installed
              1 = afterburner installed
  augmethod
              0 = afterburner activated by property /engines/engine[n]/augmentation
              1 = afterburner activated by pushing throttle above 99% position
              2 = throttle range is expanded in the FCS, and values above 1.0 are afterburner range
  injected
              0 = Water injection not installed
              1 = Water injection installed
  injection-time - Time, in seconds, of water injection duration 
</pre>

<h3>NOTES:</h3>  
<pre>
    Bypass ratio is used only to estimate engine acceleration time.  The
    effect of bypass ratio on engine efficiency is already included in
    the TSFC value.  Feel free to set this parameter (even for turbojets) to
    whatever value gives a desired spool-up rate. Default value is 0.

    The bleed factor is multiplied by thrust to give a resulting thrust
    after losses.  This can represent losses due to bleed, or any other cause.
    Default value is 0.  A common value would be 0.04.

    Nozzle position, for variable area exhaust nozzles, is provided for users
    needing to drive a nozzle gauge or animate a virtual nozzle.

    This model can only be used with the "direct" thruster.  See the file:
    /engine/direct.xml
</pre>
    @author David P. Culp
    @version "$Id: FGTurbine.h,v 1.18 2009/10/24 22:59:30 jberndt Exp $"
*/

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
CLASS DECLARATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

class FGTurbine : public FGEngine
{
public:
  /** Constructor
      @param Executive pointer to executive structure
      @param el pointer to the XML element representing the turbine engine
      @param engine_number engine number  */
  FGTurbine(FGFDMExec* Executive, Element *el, int engine_number);
  /// Destructor
  ~FGTurbine();

  enum phaseType { tpOff, tpRun, tpSpinUp, tpStart, tpStall, tpSeize, tpTrim };

  double Calculate(void);
  double CalcFuelNeed(void);
  double GetPowerAvailable(void);
  /** A lag filter.
      Used to control the rate at which values are allowed to change.
      @param var a pointer to a variable of type double
      @param target the desired (target) value
      @param accel the rate, per second, the value may increase
      @param decel the rate, per second, the value may decrease    */
  double Seek(double* var, double target, double accel, double decel);

  phaseType GetPhase(void) { return phase; }

  bool GetOvertemp(void)  const {return Overtemp; }
  bool GetInjection(void) const {return Injection;}
  bool GetFire(void) const { return Fire; }
  bool GetAugmentation(void) const {return Augmentation;}
  bool GetReversed(void) const { return Reversed; }
  bool GetCutoff(void) const { return Cutoff; }
  int GetIgnition(void) const {return Ignition;}

  double GetInlet(void) const { return InletPosition; }
  double GetNozzle(void) const { return NozzlePosition; }
  double GetBleedDemand(void) const {return BleedDemand;}
  double GetN1(void) const {return N1;}
  double GetN2(void) const {return N2;}
  double GetEPR(void) const {return EPR;}
  double GetEGT(void) const {return EGT_degC;}

  double getOilPressure_psi () const {return OilPressure_psi;}
  double getOilTemp_degF (void) {return KelvinToFahrenheit(OilTemp_degK);}

  void SetInjection(bool injection) {Injection = injection;}
  void SetIgnition(int ignition) {Ignition = ignition;}
  void SetAugmentation(bool augmentation) {Augmentation = augmentation;}
  void SetPhase( phaseType p ) { phase = p; }
  void SetEPR(double epr) {EPR = epr;}
  void SetBleedDemand(double bleedDemand) {BleedDemand = bleedDemand;}
  void SetReverse(bool reversed) { Reversed = reversed; }
  void SetCutoff(bool cutoff) { Cutoff = cutoff; }
  int InitRunning(void);
  void ResetToIC(void);

  std::string GetEngineLabels(const std::string& delimiter);
  std::string GetEngineValues(const std::string& delimiter);

private:

  phaseType phase;         ///< Operating mode, or "phase"
  double MilThrust;        ///< Maximum Unaugmented Thrust, static @ S.L. (lbf)
  double MaxThrust;        ///< Maximum Augmented Thrust, static @ S.L. (lbf)
  double BypassRatio;      ///< Bypass Ratio
  double TSFC;             ///< Thrust Specific Fuel Consumption (lbm/hr/lbf)
  double ATSFC;            ///< Augmented TSFC (lbm/hr/lbf)
  double IdleN1;           ///< Idle N1
  double IdleN2;           ///< Idle N2
  double N1;               ///< N1
  double N2;               ///< N2
  double N2norm;           ///< N2 normalized (0=idle, 1=max)
  double MaxN1;            ///< N1 at 100% throttle
  double MaxN2;            ///< N2 at 100% throttle
  double IdleFF;           ///< Idle Fuel Flow (lbm/hr)
  double delay;            ///< Inverse spool-up time from idle to 100% (seconds)
  double dt;               ///< Simulator time slice
  double N1_factor;        ///< factor to tie N1 and throttle
  double N2_factor;        ///< factor to tie N2 and throttle
  double ThrottlePos;      ///< FCS-supplied throttle position
  double AugmentCmd;       ///< modulated afterburner command (0.0 to 1.0)
  double TAT;              ///< total air temperature (deg C)
  double N1_spinup;        ///< N1 spin up rate from starter (per second)
  double N2_spinup;        ///< N2 spin up rate from starter (per second)
  bool Stalled;            ///< true if engine is compressor-stalled
  bool Seized;             ///< true if inner spool is seized
  bool Overtemp;           ///< true if EGT exceeds limits
  bool Fire;               ///< true if engine fire detected
  bool Injection;
  bool Augmentation;
  bool Reversed;
  bool Cutoff;
  int Injected;            ///< = 1 if water injection installed
  int Ignition;
  int Augmented;           ///< = 1 if augmentation installed
  int AugMethod;           ///< = 0 if using property /engine[n]/augmentation
                           ///< = 1 if using last 1% of throttle movement
                           ///< = 2 if using FCS-defined throttle
  double EGT_degC;
  double EPR;
  double OilPressure_psi;
  double OilTemp_degK;
  double BleedDemand;
  double InletPosition;
  double NozzlePosition;
  double correctedTSFC;
  double InjectionTimer;
  double InjectionTime;

  double Off(void);
  double Run();
  double SpinUp(void);
  double Start(void);
  double Stall(void);
  double Seize(void);
  double Trim();

  FGFunction *IdleThrustLookup;
  FGFunction *MilThrustLookup;
  FGFunction *MaxThrustLookup;
  FGFunction *InjectionLookup;

  bool Load(FGFDMExec *exec, Element *el);
  void bindmodel(void);
  void Debug(int from);

};
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#endif
