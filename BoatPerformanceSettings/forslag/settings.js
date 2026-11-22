// Inställningar: två flikar.
//  - Visning (dash): GET/POST /config, öppen för alla.
//  - Givare (ESP):   GET/POST /esp/config, kräver PIN i headern X-Pin.
//
// Fälten hittas via data-cfg="dash" / data-cfg="esp" i HTML:en, så ett nytt fält
// behöver bara läggas till där (och i DEFAULTS/LIMITS på Pi:n).

const $ = (s) => document.querySelector(s);
const $$ = (s) => [...document.querySelectorAll(s)];

const STATUS_INTERVAL = 500;   // ms, live-värden på givarfliken
const LINK_TIMEOUT = 3000;     // ms utan nytt "ms" från ESP:n = ingen kontakt

let pin = load('espPin') || '';
let espVer = null;             // versionen Pi:n har sparat
let live = {};                 // senaste /status
let lastMs = null, lastMsAt = 0;
let statusTimer = null;

// ---------- hjälpfunktioner ----------

// sessionStorage: PIN:en glöms när fliken/webbläsaren stängs
function load(k) { try { return sessionStorage.getItem(k); } catch { return null; } }
function store(k, v) { try { v === null ? sessionStorage.removeItem(k) : sessionStorage.setItem(k, v); } catch { } }

function toast(text, ok = true) {
    const t = $('#toast');
    t.textContent = text;
    t.classList.toggle('error', !ok);
    t.classList.add('show');
    clearTimeout(toast.timer);
    toast.timer = setTimeout(() => t.classList.remove('show'), ok ? 2000 : 4000);
}

function labelOf(el) {
    const lbl = el.closest('.field')?.querySelector('.field-label');
    const sec = el.closest('.section')?.querySelector('.section-title');
    const s = sec ? sec.firstChild.textContent.trim() + ': ' : '';
    return s + (lbl ? lbl.textContent.trim() : el.id);
}

function fill(group, data) {
    for (const el of $$(`[data-cfg="${group}"]`)) {
        if (el.id in data && data[el.id] !== null) el.value = data[el.id];
    }
}

// Läser alla fält i gruppen, kastar fel med begripligt meddelande
function collect(group) {
    const out = {};
    for (const el of $$(`[data-cfg="${group}"]`)) {
        el.classList.remove('invalid');
        const v = parseFloat(el.value);
        const bad = el.validity.badInput || el.validity.rangeUnderflow || el.validity.rangeOverflow; // step kontrolleras inte
        if (el.value === '' || isNaN(v) || bad) {
            el.classList.add('invalid');
            el.focus();
            throw new Error(`${labelOf(el)}: ogiltigt värde (${el.min}–${el.max})`);
        }
        out[el.id] = v;
    }
    return out;
}

async function api(url, opts = {}) {
    const headers = { 'Content-Type': 'application/json', ...(opts.headers || {}) };
    const r = await fetch(url, { ...opts, headers, signal: AbortSignal.timeout(3000) });
    let body = {};
    try { body = await r.json(); } catch { }
    if (!r.ok) {
        const err = new Error(body.error || `Servern svarade ${r.status}`);
        err.status = r.status;
        throw err;
    }
    return body;
}

// ---------- flikar ----------

function showTab(name) {
    $$('.tab').forEach((t) => t.classList.toggle('active', t.dataset.tab === name));
    $('#panel_dash').classList.toggle('hidden', name !== 'dash');
    $('#panel_esp').classList.toggle('hidden', name !== 'esp');
    updatePolling();
}

function updatePolling() {
    const run = !$('#panel_esp').classList.contains('hidden') && pin !== '';
    if (run && !statusTimer) {
        pollStatus();
        statusTimer = setInterval(pollStatus, STATUS_INTERVAL);
    } else if (!run && statusTimer) {
        clearInterval(statusTimer);
        statusTimer = null;
    }
}

// ---------- visning (dash) ----------

async function loadDash() {
    try {
        fill('dash', await api('/config'));
    } catch (e) {
        toast('Kunde inte hämta inställningar', false);
    }
}

async function saveDash() {
    try {
        const cfg = collect('dash');
        if (cfg.lift_min >= cfg.lift_max) throw new Error('Lyft: undre gräns måste vara lägre än övre');
        if (cfg.slip_warn >= cfg.slip_bad) throw new Error('Slip: varning måste vara lägre än larm');
        await api('/config', { method: 'POST', body: JSON.stringify(cfg) });
        toast('Visning sparad');
    } catch (e) {
        toast(e.message, false);
    }
}

// ---------- givare (ESP) ----------

function setLocked(locked) {
    $('#esp_locked').classList.toggle('hidden', !locked);
    $('#esp_unlocked').classList.toggle('hidden', locked);
    $('#lock_icon').innerHTML = locked ? '&#128274;' : '&#128275;';
    updatePolling();
}

async function unlock(p) {
    try {
        const cfg = await api('/esp/config', { headers: { 'X-Pin': p } });
        pin = p;
        store('espPin', p);
        espVer = cfg.ver ?? null;
        fill('esp', cfg);
        setLocked(false);
    } catch (e) {
        pin = '';
        store('espPin', null);
        setLocked(true);
        if (p) toast(e.status === 401 ? 'Fel PIN-kod' : e.message, false);
    }
}

function lock() {
    pin = '';
    store('espPin', null);
    $('#esp_pin').value = '';
    setLocked(true);
}

async function saveEsp() {
    try {
        const cfg = collect('esp');
        if (Math.abs(cfg.trim_ratio_b - cfg.trim_ratio_a) < 0.01) throw new Error('Trim: kvoterna nere och uppe ligger för nära varandra');
        if (Math.abs(cfg.tank_ratio_b - cfg.tank_ratio_a) < 0.01) throw new Error('Tank: kvoterna tom och full ligger för nära varandra');
        const r = await api('/esp/config', { method: 'POST', headers: { 'X-Pin': pin }, body: JSON.stringify(cfg) });
        espVer = r.ver ?? espVer;
        toast('Sparad, skickas till ESP');
        renderSync();
    } catch (e) {
        if (e.status === 401) lock();
        toast(e.message, false);
    }
}

async function resetLift() {
    if (!confirm('Nollställ lyftpositionen? Gör det bara när motorn står i nolläget.')) return;
    try {
        await api('/esp/reset_lift', { method: 'POST', headers: { 'X-Pin': pin } });
        toast('Lyft nollställd');
    } catch (e) {
        toast(e.message, false);
    }
}

// "Läs in": kopiera aktuellt råvärde till fältet
function learn(btn) {
    const v = live[btn.dataset.learn];
    if (typeof v !== 'number') {
        toast('Inget värde från ESP:n än', false);
        return;
    }
    const el = document.getElementById(btn.dataset.target);
    const dec = (el.step.split('.')[1] || '').length;
    el.value = v.toFixed(dec);
    el.classList.add('learned');
    setTimeout(() => el.classList.remove('learned'), 800);
}

async function pollStatus() {
    try {
        live = await api('/status');
    } catch {
        live = {};
    }
    if (typeof live.ms === 'number' && live.ms !== lastMs) {
        lastMs = live.ms;
        lastMsAt = Date.now();
    }
    for (const el of $$('[data-live]')) {
        const v = live[el.dataset.live];
        el.textContent = typeof v === 'number' ? v.toFixed(+el.dataset.dec) : '—';
    }
    renderSync();
}

function renderSync() {
    const box = $('#esp_sync');
    let state, text;
    if (Date.now() - lastMsAt > LINK_TIMEOUT) {
        state = 'bad';  text = 'Ingen kontakt med ESP:n';
    } else if (live.cfg === undefined) {
        state = 'warn'; text = 'ESP:ns firmware rapporterar ingen config-version';
    } else if (espVer !== null && live.cfg === espVer) {
        state = 'ok';   text = `ESP synkad (version ${espVer})`;
    } else {
        state = 'warn'; text = `Skickar till ESP… (ESP ${live.cfg}, Pi ${espVer})`;
    }
    box.dataset.state = state;
    $('#esp_sync_text').textContent = text;
}

// ---------- start ----------

document.addEventListener('DOMContentLoaded', () => {
    $$('.tab').forEach((t) => t.addEventListener('click', () => showTab(t.dataset.tab)));
    $('#save_dash').addEventListener('click', saveDash);
    $('#save_esp').addEventListener('click', saveEsp);
    $('#unlock_esp').addEventListener('click', () => unlock($('#esp_pin').value.trim()));
    $('#esp_pin').addEventListener('keydown', (e) => { if (e.key === 'Enter') unlock(e.target.value.trim()); });
    $('#lock_esp').addEventListener('click', lock);
    $('#reset_lift').addEventListener('click', resetLift);
    $$('.btn_learn').forEach((b) => b.addEventListener('click', () => learn(b)));
    $$('.field-input').forEach((el) => el.addEventListener('input', () => el.classList.remove('invalid')));

    loadDash();
    if (pin) unlock(pin); else setLocked(true);
    if (location.hash === '#givare') showTab('esp');
});
