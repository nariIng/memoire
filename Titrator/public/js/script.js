let vol = [];
let pH = [];
let d_ph = [];
let titre = 0;
let pHGraphique = null;
let dPhGraphiqueInstance = null;

document.addEventListener('DOMContentLoaded', () => {
  const links = document.querySelectorAll('a');
  links.forEach(link => {
    link.addEventListener('click', (event) => {
      const target = event.target.getAttribute('href');
      if (target && !target.startsWith('#') && !target.startsWith('mailto') && !target.startsWith('tel')) {
        event.preventDefault();
        document.body.classList.add('fade-out');
        setTimeout(() => {
          window.location.href = target;
        }, 1); // Correspond à la durée de la transition CSS
      }
    });
  });

  document.body.classList.add('fade-in');
});

function dPhGraphique(d_ph, vol) {
  const ctx = document.getElementById('derivative-curve').getContext('2d');

  const data = {
    labels: vol,
    datasets: [{
      label: 'dpH/dv en fonction du Volume',
      data: d_ph,
      borderColor: 'rgb(6, 78, 42)',
      borderWidth: 2,
      fill: false,
    }]
  };

  const config = {
    type: 'line',
    data: data,
    options: {
      responsive: true,
      scales: {
        x: {
          title: {
            display: true,
            text: 'Volume',
            color: '#ffffff' // Assurez-vous que cette couleur est bien appliquée
          },
          ticks: {
            color: '#ffffff' // Assurez-vous que cette couleur est bien appliquée
          },
          grid: {
            color: 'rgba(255, 255, 255, 0.2)' // Optionnel : couleur de la grille
          }
        },
        y: {
          title: {
            display: true,
            text: 'dpH/dv',
            color: '#ffffff' // Assurez-vous que cette couleur est bien appliquée
          },
          ticks: {
            color: '#ffffff' // Assurez-vous que cette couleur est bien appliquée
          },
          grid: {
            color: 'rgba(255, 255, 255, 0.2)' // Optionnel : couleur de la grille
          }
        }
      },
      plugins: {
        legend: {
          labels: {
            color: '#ffffff' // Couleur des labels de la légende
          }
        }
      }
    }
  };

  if (window.dPhGraphiqueInstance) {
    window.dPhGraphiqueInstance.data.labels = vol;
    window.dPhGraphiqueInstance.data.datasets[0].data = d_ph;
    window.dPhGraphiqueInstance.update();
  } else {
    window.dPhGraphiqueInstance = new Chart(ctx, config);
  }
}

function creerGraphique(pH, vol) {
  const ctx = document.getElementById('titration-curve').getContext('2d');

  const data = {
    labels: vol,
    datasets: [{
      label: 'pH en fonction du Volume',
      data: pH,
      borderColor: '#0c437a',
      borderWidth: 2,
      fill: false,
    }]
  };

  const config = {
    scaleFontColor: "white",
    type: 'line',
    data: data,
    options: {
      responsive: true,
      scales: {
        x: {
          title: {
            display: true,
            text: 'Volume',
            color: '#ffffff'
          },
          ticks: {
            color: '#ffffff', // Assurez-vous que cette option est bien prise en charge
            font: {
              size: 12, // Optionnel : ajustez la taille de la police
            }
          },
          grid: {
            color: 'rgba(255, 255, 255, 0.2)' // Optionnel
          }
        },
        y: {
          title: {
            display: true,
            text: 'pH',
            color: '#ffffff'
          },
          ticks: {
            color: '#ffffff', // Assurez-vous que cette option est bien prise en charge
            font: {
              size: 12, // Optionnel : ajustez la taille de la police
            }
          },
          grid: {
            color: 'rgba(255, 255, 255, 0.2)' // Optionnel
          }
        }
      }
    }
  };
  

  if (pHGraphique) {
    pHGraphique.data.labels = vol;
    pHGraphique.data.datasets[0].data = pH;
    pHGraphique.update();
  } else {
    pHGraphique = new Chart(ctx, config);
  }
}

async function fetchph() {
  try {
    const [res_ph, res_pound_1, res_pound_2, res_start_send] = await Promise.all([
      fetch('/ph'),
      fetch('/pound_1'),
      fetch('/pound_2'),
      fetch('/start_send')
    ]);

    const ph_meter = await res_ph.text();
    const pound_1 = await res_pound_1.text();
    const pound_2 = await res_pound_2.text();
    titre = await res_start_send.json();

    if (titre == 1) {
      const [res_vol_1, res_vol_2] = await Promise.all([
        fetch('/vol_1'),
        fetch('/vol_2')
      ]);
      const vol_1 = await res_vol_1.text();
      const vol_2 = await res_vol_2.text();
      document.getElementById('real-time-vol-1').innerText = vol_1;
      document.getElementById('real-time-vol-2').innerText = vol_2;

      const [phData, volData, d_phData] = await Promise.all([
        fetch('/api/ph'),
        fetch('/api/vol'),
        fetch('/api/d_ph')
      ]);

      pH = await phData.json();
      vol = await volData.json();
      d_ph = await d_phData.json();
      let vol_temp = vol;
      creerGraphique(pH, vol);
      dPhGraphique(d_ph, vol);     

    }

    document.getElementById('real-time-ph').innerText = ph_meter;
    document.getElementById('real-time-pound-1').innerText = pound_1;
    document.getElementById('real-time-pound-2').innerText = pound_2;
  } catch (error) {
    console.error('Erreur lors de la récupération des données:', error);
  }
}

function start_titre() {
  fetch(`/start_receiv?text=${titre}`);
}

function end_titre() {
  titre = 0;
  fetch(`/start_receiv?text=${titre}`);
}

async function sendMessage(event) {
  event.preventDefault();

  const titrant = document.getElementById('titrant-volume').value;
  const titrand = document.getElementById('titrand-volume').value;
  const pD1 = document.getElementById('real-time-pound-1').innerText;
  const pD2 = document.getElementById('real-time-pound-2').innerText;

  titre = 1;
  start_titre();

  try {
    const [titrandResponse, titrantResponse, poundResponse1, poundResponse2] = await Promise.all([
      fetch(`/titrant?text=${titrant}`),
      fetch(`/titrand?text=${titrand}`),
      fetch(`/pD1?text=${pD1}`),
      fetch(`/pD2?text=${pD2}`)
    ]);

    if (!titrandResponse.ok) {
      console.error('Erreur lors de l\'envoi de titrand:', titrandResponse.statusText);
    }

    if (!titrantResponse.ok) {
      console.error('Erreur lors de l\'envoi de titrant:', titrantResponse.statusText);
    }
  } catch (error) {
    console.error('Erreur lors de l\'envoi des données:', error);
  }
}

async function pompSet(event) {
  event.preventDefault();
  const pumpSpeed = document.getElementById('pumpSpeed').value;

  try {
    const pumpResponse = await fetch(`/pump?text=${pumpSpeed}`);
    if (!pumpResponse.ok) {
      console.error('Erreur lors de l\'envoi de la vitesse de la pompe:', pumpResponse.statusText);
    }
  } catch (error) {
    console.error('Erreur lors de l\'envoi des données:', error);
  }
}

document.getElementById('titration-form').addEventListener('input', () => {
  document.getElementById('display-titrant-name').textContent = document.getElementById('titrant-name').value || '-';
  document.getElementById('display-titrand-name').textContent = document.getElementById('titrand-name').value || '-';
  document.getElementById('display-titrant-volume').textContent = `${document.getElementById('titrant-volume').value || '0'} mL`;
  document.getElementById('display-titrand-volume').textContent = `${document.getElementById('titrand-volume').value || '0'} mL`;
});

setInterval(fetchph, 1000);
