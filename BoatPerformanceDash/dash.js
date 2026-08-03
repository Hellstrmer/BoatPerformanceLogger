/* ============================================================
   Hydrolift T18 — instrumentpanel
   ============================================================ */

// ---- riggkonstanter — sätt dina riktiga värden ----
// const RPM_MAX = 6200;   // stapelns skala
// const REDLINE = 5500;   // kontrollera mot manualen
// const GEAR = 2.00;   // utväxling, verifiera för din Autolube 150 -01
// const PITCH_IN = 25;     // propellerns stigning i tum, står på proppen
// const TRIM_MIN = -5;
// const TRIM_MAX = 25;
// const LIFT_MAX = 154.2;  // mm
// const FUEL_MAX = 70;     // l/h vid fullt pådrag

const POLL_MS = 100;
const STALE_MS = 1000;   // utan svar längre än så: visa INGEN LÄNK

let cfg = {};

async function init() {
  cfg = await getSettings();

  // redline-zonen ritas en gång
  $('wzone').style.width = (100 - cfg.redline / cfg.rpm_max * 100) + '%';

  console.log(cfg.trim_max)

  setInterval(poll, POLL_MS);
  addEventListener('resize', fit);
  fit();
}

let lastEspMs = null, lastEspChange = Date.now();

// ---- härledningar ----
// farten propellern teoretiskt ger vid ett varvtal, utan slip
const knFromRpm = rpm => (rpm * cfg.prop_pitch) / (cfg.prop_gear * 1215);
// varvtalet en fart motsvarar vid noll slip — "grepp-varvtalet"
const rpmFromKn = kn => (kn * cfg.prop_gear * 1215) / cfg.prop_pitch;

const $ = id => document.getElementById(id);
const pct = v => Math.max(0, Math.min(100, v));

async function getSettings() {
  try {
    const r = await fetch('/config', { signal: AbortSignal.timeout(1000) });
    const d = await r.json();
    console.log(d);

    return d;

  } catch (e) {
    console.log("Fetch misslyckades: " + e);
  }
}

/**
 * Ritar hela panelen.
 * @param {{rpm:number, kn:number, trim:number, lift:number, fuel:number, link:boolean}} d
 */
function render(d) {
  // Visar greppmarkören.
  const hookRpm = rpmFromKn(d.kn);
  const slip = d.slip;//d.rpm > 400 ? pct((1 - d.kn / knFromRpm(d.rpm)) * 100) : 0;
  const knots = (d.kn);

  const aPct = pct(d.rpm / cfg.rpm_max * 100);   // faktiskt varvtal
  const hPct = pct(hookRpm / cfg.rpm_max * 100); // grepp-varvtal

  // stapeln visar varvtal. bara redline färgar den.
  $('wfill').style.width = aPct + '%';
  //$('wfill').style.background = d.rpm >= REDLINE ? 'var(--red)' : 'var(--white)';

  // gapet visar slip. bara slip färgar det.
  $('wgap').style.left = Math.min(hPct, aPct) + '%';
  $('wgap').style.width = Math.abs(aPct - hPct) + '%';
  $('wgap').style.background = slip >= cfg.slip_bad ? 'var(--red)' : slip < cfg.slip_warn ? 'var(--green)' : 'var(--yellow)';
  $('ghost').style.left = hPct + '%';

  $('rpm').textContent = Math.round(d.rpm).toLocaleString('sv-SE');
  $('rpm').className = d.rpm >= cfg.redline ? 'bad' : '';

  $('slip').textContent = slip.toFixed(1);
  $('slip').className = slip >= cfg.slip_bad ? 'bad' : slip < cfg.slip_warn ? 'ok' : 'warn';

  $('knots').textContent = knots.toFixed(1);

  $('trim').textContent = d.trim.toFixed(1);
  $('trimBar').style.width = pct((d.trim - cfg.trim_min) / (cfg.trim_max - cfg.trim_min) * 100) + '%';

  $('lift').textContent = Math.round(d.lift);
  $('liftBar').style.width = pct(d.lift / cfg.lift_max * 100) + '%';

  $('fuel').textContent = Math.round(d.fuel);
  $('fuelBar').style.width = pct(d.fuel / cfg.fuel_max * 100) + '%';


  $('Alarm').classList.toggle('on', d.overheat || d.oilLow);
  $('Alarm').textContent = d.overheat ? "ENGINE OVERHEAT" : d.oilLow ? "OIL PRESSURE LOW" : "";

}

let failCount = 0;
async function poll() {
  let d;
  try {
    const r = await fetch('/status', { signal: AbortSignal.timeout(3000) });
    d = await r.json();
    failCount = 0;                    // lyckades — nollställ
  } catch (e) {
    failCount++;
    if (failCount >= 3) {
      $('stale').classList.add('on');
      $('stale').textContent = "SERVER OFFLINE";
    }
    return;
  }

  const stale = checkStale(d);
  if (stale) {
    $('stale').classList.add('on');
    $('stale').textContent = "NO SIGNAL TO CONTROLLER";
    return;
  }

  $('stale').classList.remove('on');
  render(d);
}
function checkStale(d) {
  if (d.ms === undefined) return true;   // ESP har aldrig skickat
  if (d.ms !== lastEspMs) {
    lastEspMs = d.ms;
    lastEspChange = Date.now();
  }
  return Date.now() - lastEspChange > STALE_MS;
}


// ---- skala panelen till fönstret för förhandsgranskning ----
function fit() {
  //const s = Math.min(innerWidth / 1024, innerHeight / 600);
  const s = Math.min(innerWidth / 480, innerHeight / 320);
  $('dash').style.transform = 'scale(' + s + ')';
}
init();
