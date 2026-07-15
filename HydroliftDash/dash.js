/* ============================================================
   Hydrolift T18 — instrumentpanel
   ============================================================ */

// ---- riggkonstanter — sätt dina riktiga värden ----
const RPM_MAX   = 6000;   // stapelns skala
const REDLINE   = 5600;   // kontrollera mot manualen
const GEAR      = 2.00;   // utväxling, verifiera för din Autolube 150 -01
const PITCH_IN  = 25;     // propellerns stigning i tum, står på proppen
const TRIM_MIN  = -5;
const TRIM_MAX  = 25;
const LIFT_MAX  = 154.2;  // mm
const FUEL_MAX  = 70;     // l/h vid fullt pådrag
const SLIP_WARN = 12;     // % — under detta är greppet bra
const SLIP_BAD  = 18;     // % — över detta larmar den

const DEMO      = true;   // false när motornoden matar riktig data
const POLL_MS   = 100;
const STALE_MS  = 1000;   // utan svar längre än så: visa INGEN LÄNK

// ---- härledningar ----
// farten propellern teoretiskt ger vid ett varvtal, utan slip
const knFromRpm = rpm => (rpm * PITCH_IN) / (GEAR * 1215);
// varvtalet en fart motsvarar vid noll slip — "grepp-varvtalet"
const rpmFromKn = kn  => (kn * GEAR * 1215) / PITCH_IN;

const $   = id => document.getElementById(id);
const pct = v  => Math.max(0, Math.min(100, v));

// redline-zonen ritas en gång
$('wzone').style.width = (100 - REDLINE / RPM_MAX * 100) + '%';

/**
 * Ritar hela panelen.
 * @param {{rpm:number, kn:number, trim:number, lift:number, fuel:number, link:boolean}} d
 */
function render(d){
  const hookRpm = rpmFromKn(d.kn);
  const slip = d.rpm > 400 ? pct((1 - d.kn / knFromRpm(d.rpm)) * 100) : 0;
  const knots = (hookRpm / 100);

  const aPct = pct(d.rpm / RPM_MAX * 100);   // faktiskt varvtal
  const hPct = pct(hookRpm / RPM_MAX * 100); // grepp-varvtal

  // stapeln visar varvtal. bara redline färgar den.
  $('wfill').style.width      = aPct + '%';
  $('wfill').style.background = d.rpm >= REDLINE ? 'var(--red)' : 'var(--white)';

  // gapet visar slip. bara slip färgar det.
  $('wgap').style.left       = Math.min(hPct, aPct) + '%';
  $('wgap').style.width      = Math.abs(aPct - hPct) + '%';
  $('wgap').style.background = slip >= SLIP_BAD ? 'var(--red)' : slip < SLIP_WARN ? 'var(--green)' : 'var(--yellow)';
  $('ghost').style.left      = hPct + '%';

  $('rpm').textContent  = Math.round(d.rpm).toLocaleString('sv-SE');
  $('rpm').className    = d.rpm >= REDLINE ? 'warn' : '';

  $('slip').textContent = slip.toFixed(1);
  $('slip').className   = slip >= SLIP_BAD ? 'bad' : slip < SLIP_WARN ? 'ok' : 'warn';


  $('knots').textContent = knots.toFixed(1);

  $('trim').textContent    = d.trim.toFixed(1);
  $('trimBar').style.width = pct((d.trim - TRIM_MIN) / (TRIM_MAX - TRIM_MIN) * 100) + '%';

  $('lift').textContent    = Math.round(d.lift);
  $('liftBar').style.width = pct(d.lift / LIFT_MAX * 100) + '%';

  $('fuel').textContent    = Math.round(d.fuel);
  $('fuelBar').style.width = pct(d.fuel / FUEL_MAX * 100) + '%';

  $('stale').classList.toggle('on', !d.link);
}

// ---- datakälla: motornoden ----
let lastOk = 0;

async function poll(){
  try {
    const r = await fetch('/status');
    const d = await r.json();
    lastOk = Date.now();
    render({ ...d, link: true });
  } catch {
    if (Date.now() - lastOk > STALE_MS) $('stale').classList.add('on');
  }
}

// ---- demodata så panelen går att bedöma utan båt ----
let t = 0;
function demo(){
  t += POLL_MS / 1000;
  const rpm  = 3400 + Math.sin(t * .28) * 1900 + Math.sin(t * 3.1) * 40;
  const trim = 8 + Math.sin(t * .19) * 7;
  // slippet stiger i accelerationen och när trimmet går för högt, som i verkligheten
  const extra = Math.max(0, trim - 11) * 1.6 + Math.max(0, Math.sin(t * .28)) * 5;
  render({
    rpm,
    kn:   Math.max(0, knFromRpm(rpm) * (1 - (9 + extra) / 100)),
    trim,
    lift: 96 + Math.sin(t * .13) * 34,
    fuel: 8 + rpm / 5600 * 52,
    link: true
  });
}

setInterval(DEMO ? demo : poll, POLL_MS);

// ---- skala panelen till fönstret för förhandsgranskning ----
function fit(){
  const s = Math.min(innerWidth / 1024, innerHeight / 600);
  $('dash').style.transform = 'scale(' + s + ')';
}
addEventListener('resize', fit);
fit();
