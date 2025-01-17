let value = 0;

function sendvalue () {
    try {
    const sendE = fetch (`/param?text=${value}`);
    if (!sendE.ok) {
        console.error('Erreur lors de l\'envoi de titrand:', sendE.statusText);
    }
} catch (error) {
    console.error('Erreur lors de l\'envoi des données:', error);
  }
}

async function tarBalance1() {
    value = 1;
    sendvalue();
}

async function tarBalance2() {
    value = 2;
    sendvalue();
}

async function calibPH() {
    value = 3;
    sendvalue();
}

async function cleanPompe() {
    value = 4;
    sendvalue();
}
