
const propGear = document.getElementById('prop_gear')
const propPitch = document.getElementById('prop_pitch')

const engineRedline = document.getElementById('engine_redline')
const engineRPMMax = document.getElementById('engine_rpm_max')

const liftLower = document.getElementById('lift_lower')
const liftUpper = document.getElementById('lift_upper')
const resetLift = document.getElementById('reset_lift');

const trimLower = document.getElementById('trim_lower');
const trimUpper = document.getElementById('trim_upper');
const resetTrim = document.getElementById('reset_trim');

const waterLower = document.getElementById('water_lower');
const waterUpper = document.getElementById('water_upper');

const saveSettings = document.getElementById('save_settings')


document.addEventListener('DOMContentLoaded', () => {

  saveSettings.addEventListener('click', () => {

  })

})


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

setInterval(() => { fetch('/status').then(r => r.json()).then(update).catch(() => { }); }, 200);
function saveConfig() {
  const body = new FormData();
  const l = document.getElementById('lower').value;
  const u = document.getElementById('upper').value;
  if (l) body.append('lower', l);
  if (u) body.append('upper', u);
  fetch('/config', { method: 'POST', body }).then(() => {
    const t = document.getElementById('toast');
    t.classList.add('show');
    setTimeout(() => t.classList.remove('show'), 2000);
    document.getElementById('lower').value = '';
    document.getElementById('upper').value = '';
  });
}

function resetPos() {
  fetch('/reset_lift', { method: 'POST' }).then(() => {
    const t = document.getElementById('toast');
    t.textContent = 'NOLLSTÄLLD';
    t.classList.add('show');
    setTimeout(() => { t.classList.remove('show'); t.textContent = 'Sparat'; }, 2000);
  });
}