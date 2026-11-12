/* ============================================================
   Hydrolift T18 — instrumentpanel
   Tre vyer (A, B, C). Swipa vänster/höger för att byta,
   eller piltangenterna på ett tangentbord.
   ============================================================ */

const POLL_MS = 100;      // hur ofta /status hämtas
const TEXT_MS = 200;      // hur ofta siffrorna skrivs om (5 Hz = lugnt att läsa)
const STALE_MS = 1000;    // ESP tyst längre än så = ingen signal
const SHOW_STALE = false; // sätt true när länkövervakningen ska vara aktiv

const PAGES = ['A', 'B', 'C'];
const DEFAULT_PAGE = 2;   // C

// Utjämning: 0 = står still, 1 = ingen utjämning.
const SMOOTH = { rpm: 0.5, kn: 0.35, slip: 0.3, trim: 0.4, lift: 0.4, fuel: 0.25, water: 0.3, batt: 0.2, tank: 0.1, roll: 0.4, pitch: 0.4 };

// Standardvärden för det som inte finns i /config än.
const DEFAULTS = {
  water_max: 3.0,        // bar, skalans max
  water_min: 0.5,        // bar, larm under detta...
  water_alarm_rpm: 2000, // ...men bara över detta varvtal
  batt_scale_min: 11.0,
  batt_scale_max: 15.0,
  batt_warn: 12.4,       // V
  batt_low: 12.0,        // V
  batt_high: 15.0,       // V
  tank_warn: 20,         // %
  tank_low: 10,          // %
};

let cfg = {};
const s = {};            // utjämnade värden
let last = {};           // senaste rådata
let lastText = 0;

const $ = id => document.getElementById(id);
const $$ = sel => Array.from(document.querySelectorAll(sel));
const pct = v => Math.max(0, Math.min(100, v));
const num = v => (typeof v === 'number' && isFinite(v)) ? v : null;

// ---- härledningar ----
const knFromRpm = rpm => (rpm * cfg.prop_pitch) / (cfg.prop_gear * 1215);
const rpmFromKn = kn => (kn * cfg.prop_gear * 1215) / cfg.prop_pitch;

// ---- vilka område varje mätare har ----
function range(k) {
  switch (k) {
    case 'lift': return [0, cfg.lift_max];
    case 'trim': return [cfg.trim_min, cfg.trim_max];
    case 'fuel': return [0, cfg.fuel_max];
    case 'water': return [0, cfg.water_max];
    case 'batt': return [cfg.batt_scale_min, cfg.batt_scale_max];
    case 'tank': return [0, 100];
  }
  return [0, 100];
}

function frac(k) {
  const v = s[k];
  if (v == null) return 0;
  const [lo, hi] = range(k);
  return pct((v - lo) / (hi - lo) * 100);
}

// ---- formatering ----
function fmt(k, dec) {
  if (k === 'lpnm') {
    const f = s.fuel, kn = s.kn;
    return (f != null && kn != null && kn > 3) ? (f / kn).toFixed(2) : '–';
  }
  if (k === 'sats') return String(last.satellites || 0);
  const v = s[k];
  if (v == null) return '–';
  const d = dec != null ? dec : ({ kn: 1, slip: 1, trim: 1, water: 1, batt: 1 }[k] ?? 0);
  if (k === 'rpm') return (Math.round(v / 10) * 10).toLocaleString('sv-SE');
  if (k === 'lift' || k === 'fuel' || k === 'tank' || k === 'roll' || k === 'pitch') return String(Math.round(v));
  return v.toFixed(d);
}

// ---- tillstånd (färg) ----
function state(k) {
  const v = s[k];
  if (v == null) return '';
  switch (k) {
    case 'rpm': return v >= cfg.redline ? 'bad' : '';
    case 'slip':
      if ((s.rpm ?? 0) < 400) return '';
      return v >= cfg.slip_bad ? 'bad' : v < cfg.slip_warn ? 'ok' : 'warn';
    case 'water':
      return ((s.rpm ?? 0) > cfg.water_alarm_rpm && v < cfg.water_min) ? 'bad' : '';
    case 'batt':
      if (v < cfg.batt_low || v > cfg.batt_high) return 'bad';
      return v < cfg.batt_warn ? 'warn' : '';
    case 'tank':
      return v < cfg.tank_low ? 'bad' : v < cfg.tank_warn ? 'warn' : '';
  }
  return '';
}

// ---- cache av element ----
let E = {};
function bind() {
  E = {
    text: $$('[data-v]'),
    fill: $$('[data-fill]'),
    vfill: $$('[data-vfill]'),
    blk: $$('[data-blk]'),
    state: $$('[data-state]'),
    wfill: $$('.wfill'), wgap: $$('.wgap'), wzone: $$('.wzone'), ghost: $$('.ghost'),
    tiltDot: $$('.tiltDot'), crossH: $$('.crossH'), crossV: $$('.crossV'),
    gpsDot: $$('.gpsDot'),
  };

  // bygg blocken
  E.blk.forEach(el => {
    const n = +el.dataset.n || 10;
    el.innerHTML = '<i></i>'.repeat(n);
    el._i = Array.from(el.children);
  });

  // skalor (min / mitt / max) från config
  $$('[data-scale]').forEach(el => {
    const [lo, hi] = range(el.dataset.scale);
    const mid = (lo + hi) / 2;
    const f = v => (Math.abs(v) < 10 && v % 1 !== 0) ? v.toFixed(1) : String(Math.round(v));
    el.innerHTML = `<span>${f(lo)}</span><span>${f(mid)}</span><span>${f(hi)}</span>`;
  });

  // redline-zonen ritas en gång
  E.wzone.forEach(el => el.style.width = pct(100 - cfg.redline / cfg.rpm_max * 100) + '%');
}

// ---- ritning ----
function render() {
  // RPM-kilen
  const rpm = s.rpm ?? 0, kn = s.kn ?? 0, slip = s.slip ?? 0;
  const aPct = pct(rpm / cfg.rpm_max * 100);
  const hPct = pct(rpmFromKn(kn) / cfg.rpm_max * 100);
  const gapCol = rpm < 400 ? 'transparent'
    : slip >= cfg.slip_bad ? 'var(--red)' : slip < cfg.slip_warn ? 'var(--green)' : 'var(--yellow)';
  E.wfill.forEach(el => el.style.width = aPct + '%');
  E.wgap.forEach(el => {
    el.style.left = Math.min(hPct, aPct) + '%';
    el.style.width = Math.abs(aPct - hPct) + '%';
    el.style.background = gapCol;
  });
  E.ghost.forEach(el => el.style.left = hPct + '%');

  // staplar
  E.fill.forEach(el => el.style.width = frac(el.dataset.fill) + '%');
  E.vfill.forEach(el => el.style.height = frac(el.dataset.vfill) + '%');
  E.blk.forEach(el => {
    const k = el.dataset.blk;
    const n = el._i.length;
    const f = frac(k);
    let on = Math.round(f / 100 * n);
    if (s[k] != null && f > 0 && on === 0) on = 1;   // visa alltid minst ett block om värdet finns
    el._i.forEach((b, i) => b.classList.toggle('on', i < on));
  });

  // färger
  E.state.forEach(el => {
    const st = state(el.dataset.state);
    el.classList.remove('ok', 'warn', 'bad');
    if (st) el.classList.add(st);
  });

  // lutning
  const MAXDEG = 40, TR = 95;
  const tx = 120 + ((s.roll ?? 0) / MAXDEG) * TR;
  const ty = 120 + ((s.pitch ?? 0) / MAXDEG) * TR;
  E.tiltDot.forEach(el => { el.setAttribute('cx', tx); el.setAttribute('cy', ty); });
  E.crossH.forEach(el => { el.setAttribute('x1', tx - 60); el.setAttribute('x2', tx + 60); el.setAttribute('y1', ty); el.setAttribute('y2', ty); });
  E.crossV.forEach(el => { el.setAttribute('y1', ty - 60); el.setAttribute('y2', ty + 60); el.setAttribute('x1', tx); el.setAttribute('x2', tx); });

  // GPS
  E.gpsDot.forEach(el => el.classList.toggle('fix', (last.satellites || 0) > 0));

  // siffror: bara 5 gånger per sekund, så de inte flimrar
  const now = Date.now();
  if (now - lastText >= TEXT_MS) {
    lastText = now;
    E.text.forEach(el => {
      const dec = el.dataset.dec != null ? +el.dataset.dec : null;
      const t = fmt(el.dataset.v, dec);
      if (el.textContent !== t) el.textContent = t;
    });
  }

  // larm
  const alarm = $('Alarm');
  const msg = last.overheat ? 'MOTORN<br>ÖVERHETTAD' : last.oilLow ? 'OLJETRYCK<br>LÅGT' : '';
  alarm.classList.toggle('on', !!msg);
  if (alarm.innerHTML !== msg) alarm.innerHTML = msg;
}

// ---- data ----
function ingest(d) {
  last = d;
  const raw = {
    rpm: num(d.rpm), kn: num(d.kn), slip: num(d.slip), trim: num(d.trim), lift: num(d.lift),
    fuel: num(d.fuel), water: num(d.waterpressure),
    batt: num(d.battery ?? d.vbat ?? d.batt),
    tank: num(d.tank_level),
    roll: num(d.roll), pitch: num(d.pitch),
  };
  for (const k in raw) {
    const v = raw[k];
    if (v == null) { s[k] = null; continue; }
    s[k] = (s[k] == null) ? v : s[k] + (SMOOTH[k] ?? 0.3) * (v - s[k]);
  }
}

async function getSettings() {
  try {
    const r = await fetch('/config', { signal: AbortSignal.timeout(1000) });
    return await r.json();
  } catch (e) {
    console.log('Hämtning av /config misslyckades: ' + e);
    return {};
  }
}

let failCount = 0, lastEspMs = null, lastEspChange = Date.now();

function checkStale(d) {
  if (d.ms === undefined) return true;
  if (d.ms !== lastEspMs) { lastEspMs = d.ms; lastEspChange = Date.now(); }
  return Date.now() - lastEspChange > STALE_MS;
}

async function poll() {
  let d;
  try {
    const r = await fetch('/status', { signal: AbortSignal.timeout(3000) });
    d = await r.json();
    failCount = 0;
  } catch (e) {
    failCount++;
    if (SHOW_STALE && failCount >= 3) {
      $('stale').textContent = 'SERVERN SVARAR INTE';
      $('stale').classList.add('on');
    }
    return;
  }
  if (SHOW_STALE && checkStale(d)) {
    $('stale').textContent = 'INGEN SIGNAL FRÅN STYRENHETEN';
    $('stale').classList.add('on');
    return;
  }
  $('stale').classList.remove('on');
  ingest(d);
  render();
}

// ---- vyer och swipe ----
let page = DEFAULT_PAGE, dotsTimer = null;

function showPage(i, animate = true) {
  page = (i + PAGES.length) % PAGES.length;
  const p = $('pages');
  p.style.transition = animate ? '' : 'none';
  p.style.transform = `translateX(${-page * 1280}px)`;
  try { localStorage.setItem('dashPage', String(page)); } catch (e) { }

  const dots = $('dots');
  dots.innerHTML = PAGES.map((n, j) => `<span class="${j === page ? 'cur' : ''}">${n}</span>`).join('');
  dots.classList.add('on');
  clearTimeout(dotsTimer);
  dotsTimer = setTimeout(() => dots.classList.remove('on'), 1500);
}

function initSwipe() {
  let x0 = null, y0 = 0, t0 = 0;
  addEventListener('pointerdown', e => { x0 = e.clientX; y0 = e.clientY; t0 = Date.now(); });
  addEventListener('pointerup', e => {
    if (x0 === null) return;
    const dx = e.clientX - x0, dy = e.clientY - y0, dt = Date.now() - t0;
    x0 = null;
    const min = innerWidth * 0.12;              // minst 12 % av skärmbredden
    if (Math.abs(dx) > min && Math.abs(dx) > Math.abs(dy) * 1.5 && dt < 800) {
      showPage(page + (dx < 0 ? 1 : -1));
    }
  });
  addEventListener('pointercancel', () => { x0 = null; });
  addEventListener('keydown', e => {
    if (e.key === 'ArrowRight') showPage(page + 1);
    if (e.key === 'ArrowLeft') showPage(page - 1);
  });
  // stoppa webbläsarens egna gester (bakåt/zoom)
  addEventListener('touchmove', e => e.preventDefault(), { passive: false });
  addEventListener('contextmenu', e => e.preventDefault());
}

// ---- skala panelen till skärmen ----
function fit() {
  const sc = Math.min(innerWidth / 1280, innerHeight / 720);
  $('dash').style.transform = 'scale(' + sc + ')';
}

async function init() {
  cfg = Object.assign({}, DEFAULTS, await getSettings());
  bind();

  let saved = null;
  try { saved = localStorage.getItem('dashPage'); } catch (e) { }
  showPage(saved !== null ? +saved : DEFAULT_PAGE, false);
  $('dots').classList.remove('on');

  initSwipe();
  addEventListener('resize', fit);
  fit();
  setInterval(poll, POLL_MS);
}

init();
