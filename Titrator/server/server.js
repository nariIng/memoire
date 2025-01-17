const express = require('express');
const path = require('path');
const axios = require('axios');
const os = require('os');

const app = express();
const port = 3000;

let pH = [];
let volume = [];
let volume_cpy = [];
let d_pH = [];
let volume2 = [];
let pD1 = 0;
let pD2 = 0;
let vol1 = 0;
let vol2 = 0;
let titre = 0;

// Cache de données temporaire avec une expiration
const cache = {
  pH: { data: [], timestamp: null },
  d_pH: { data: [], timestamp: null },
  volume: { data: [], timestamp: null },
  volume2: { data: [], timestamp: null },
  expireTime: 5000, // 5 seconds
};

app.use(express.static(path.join(__dirname, '../public')));
app.use(express.json()); // Pour parser les JSON dans les requêtes

// Utilisation d'un timeout et gestion des erreurs pour les requêtes axios
const axiosInstance = axios.create({
  timeout: 5000, // Timeout de 5 secondes
});

//envoyer les nouveau tableau pH
app.get('/api/ph', async (req, res) => {
    res.json(pH);
});

app.get('/api/vol', (req, res) => {
  res.json(volume);
});

app.get('/api/d_ph', (req, res) => {
  console.log(d_pH);
  res.json(d_pH);
})

app.get('/start_send', (req, res) => {
  res.json(titre);
})

//recevoir donnees pH
app.get('/ph', async (req, res) => {
    try {
      const response = await axiosInstance.get('http://192.168.4.1/ph');
      let ph = response.data + 1;
      ph = ph.toFixed(2);
      pH.push(ph);     
      cache.pH = { data: pH, timestamp: Date.now() }; // Mettre en cache

      if (volume.length > 1) {
        let deltaVolume = volume[volume.length - 1] - volume[volume.length - 2];
        if (deltaVolume !== 0) {
          let d_ph_val = (pH[pH.length - 1] - pH[pH.length - 2]) / deltaVolume;
          console.log(d_ph_val);
          if (d_ph_val != NaN)
            d_pH.push(d_ph_val);
          else{
            d_pH.push(0);
          }
        } else {
          console.warn('Delta de volume est zéro ou invalide.');
          d_pH.push(0);
        }
      } else {
        console.warn('Pas assez de données pour calculer d_ph_val.');
        d_pH.push(0);
      }

      res.send((ph).toString());
      // cache.d_pH = { data: d_pH, timestamp: Date.now() }; // Mettre en cache
    } catch (error) {
      console.error('Erreur lors de la récupération du pH:', error);
      res.status(500).send('0');
    }
});

function vol_send1 (pound){
  let vol = (vol1 - (pound * vol1 / pD1)).toFixed(2);
  volume.push(vol);
  cache.volume = { data: volume, timestamp: Date.now() }; // Mettre en cache

}

function vol_send2 (pound){
  let vol = pound * vol2 / pD2;
  volume2.push(vol.toFixed(2));
}

app.get('/vol_1', async (req, res) => {
  let a = (vol1 - volume[volume.length -1]).toFixed(2);
  res.send(a.toString());
})

app.get('/vol_2', async (req, res) => {
  res.send(volume2[volume2.length - 1].toString());
})

//recevoir donnees titrant
app.get('/pound_1', async (req, res) => {
    try {
        const response = await axiosInstance.get('http://192.168.4.1/vol_1');
        let pound_1 = response.data;
        vol_send1(pound_1);
        res.send(pound_1.toString());
    } catch (error) {
        console.error('Erreur lors de la récupération du poids:', error);
        res.status(500).send('0');
    }
});

//recevoir donnees titrand
app.get('/pound_2', async (req, res) => {
    try {
        const response = await axiosInstance.get('http://192.168.4.1/vol_2');
        let pound_2 = response.data;
        vol_send2(pound_2);
        res.send(pound_2.toString());
    } catch (error) {
        console.error('Erreur lors de la récupération du poids:', error);
        res.status(500).send('0');
    }
});

//Envoyer le volume du titrant
app.get('/titrant', async (req, res) => {
  const titrant = req.query.text;
  vol1 = titrant;
  console.log(titrant);
  try {
    await axiosInstance.get(`http://192.168.4.1/titrant?text=${titrant}`);
    res.send('Message envoyé');
  } catch (error) {
    res.status(500).send('Erreur d\'envoi du message');
  }
});

//Envoyer le volume du titrand
app.get('/titrand', async (req, res) => {
  const titrand = req.query.text;
  vol2 = titrand;
  console.log(titrand);
  try {
    await axiosInstance.get(`http://192.168.4.1/titrand?text=${titrand}`);
    res.send('Message envoyé');
  } catch (error) {
    res.status(500).send('Erreur d\'envoi du message');
  }
});

//recevoir le poids du titrant
app.get('/pD1', async (req, res) => {
  pD1 = req.query.text;
})

//recevoir les donner du titrand
app.get('/pD2', async (req, res) => {
  pD2 = req.query.text;
})

//Envoyer le volume de la pompe
app.get('/pump', async (req, res) => {
  const pumpSpeed = req.query.text;
  console.log(pumpSpeed);
  try {
    await axiosInstance.get(`http://192.168.4.1/pump?text=${pumpSpeed}`);
    res.send('Message envoyé');
  } catch (error) {
    res.status(500).send('Erreur d\'envoi du message');
  }
});

// commencer le titration
app.get('/start_receiv', async (req, res) => {
  titre = req.query.text;
  console.log(titre);
  try {
    if (titre == 1) {
      pH = [];
      volume = [];
      volume_cpy = [];
      d_pH = [];
      await axiosInstance.get(`http://192.168.4.1/moteur_high?text=${titre}`);
    } else {
      await axiosInstance.get(`http://192.168.4.1/moteur_low?text=${titre}`);
    }
    res.send('Commande reçue');
  } catch (error) {
    res.status(500).send('Erreur de commande');
  }
});

//send parameter value a l'esp32
app.get('/param', async (req, res) => {
  const param = req.query.text;
  try {
    await axiosInstance.get(`http://192.168.4.1/param?text=${param}`);
    res.send('Param envoyé');
  } catch (error) {
    res.status(500).send('Erreur d\'envoi de param');
  }
});

function getLocalIp() {
    const networkInterfaces = os.networkInterfaces();
    for (const interfaceName in networkInterfaces) {
      const interfaces = networkInterfaces[interfaceName];
      for (const iface of interfaces) {
        if (iface.family === 'IPv4' && !iface.internal) {
          return iface.address;
        }
      }
    }
    return 'localhost';
}

app.listen(port, () => {
    const localIp = getLocalIp();
    console.log(`Serveur Node.js en écoute sur http://localhost:${port}`);
    console.log(`Serveur Node.js lancé à http://${localIp}:${port}`);
});
