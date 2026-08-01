
const saveSettings = document.getElementById('save_settings');
const statusBar = document.getElementById('status_bar');

document.addEventListener('DOMContentLoaded', () => {
    getSettings();    
    saveSettings.addEventListener('click', () => {
        saveConfig();

    })

})

async function getSettings() {
    try {
        const r = await fetch('/config', { signal: AbortSignal.timeout(1000) });
        const d = await r.json();
        console.log(d);

        for (const [key, value] of Object.entries(d)) {
            const el = document.getElementById(key);
            if (el) el.value = value;
        }
    } catch (e) {
        console.log("Fetch misslyckades: " + e);
    }
}

async function saveConfig() {
    const config = {};
    const fields = ["prop_gear", "prop_pitch", "redline", "rpm_max", "lift_min", "lift_max", "trim_min", "trim_max", "water_min", "water_max", "fuel_min", "fuel_max"];

    for (const key of fields) {
        const el = document.getElementById(key);
        if (el) config[key] = parseFloat(el.value);
    }
    try {
        const r = await fetch('/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(config)
        });
        if (r.ok) {
            statusBar.style.background = "#00974e";
            console.log("Post lyckades!");
        } else {
            console.log("Servern svarade fel:", r.status);
            statusBar.style.background = "#ff2b2b";
        }
    } catch (e) {
        console.log("Post misslyckades: " + e);
            statusBar.style.background = "#ff2b2b";
    }
}


const MAX = 154.2;
function update(d) {
    document.getElementById('pos').textContent = d.posMM.toFixed(1);
    document.getElementById('lower-ind').textContent = 0 + ' mm';
    document.getElementById('upper-ind').textContent = 154 + ' mm';
    const fill = document.getElementById('fill');
    fill.style.width = Math.min(100, Math.max(0, d.posMM / MAX * 100)) + '%';
    fill.style.background = d.posMM > d.upper ? '#c0392b' : d.posMM < d.lower ? '#c9960a' : '#3a9e6a';
    document.getElementById('lower').placeholder = d.lower.toFixed(1) + ' mm';
    document.getElementById('upper').placeholder = d.upper.toFixed(1) + ' mm';
}

// setInterval(() => { fetch('/status').then(r => r.json()).then(update).catch(() => { }); }, 200);
// function saveConfig() {
//   const body = new FormData();
//   const l = document.getElementById('lower').value;
//   const u = document.getElementById('upper').value;
//   if (l) body.append('lower', l);
//   if (u) body.append('upper', u);
//   fetch('/config', { method: 'POST', body }).then(() => {
//     const t = document.getElementById('toast');
//     t.classList.add('show');
//     setTimeout(() => t.classList.remove('show'), 2000);
//     document.getElementById('lower').value = '';
//     document.getElementById('upper').value = '';
//   });
// }

function resetPos() {
    fetch('/reset_lift', { method: 'POST' }).then(() => {
        const t = document.getElementById('toast');
        t.textContent = 'NOLLSTÄLLD';
        t.classList.add('show');
        setTimeout(() => { t.classList.remove('show'); t.textContent = 'Sparat'; }, 2000);
    });
}