const KEY = 'very secret';

// put the values from your sensors here
const sensor_ids = {
  2970624: 0,
  2948864: 1,
  3041792: 2,
  2953216: 3
};

const functions = require('firebase-functions');

const admin = require('firebase-admin');
admin.initializeApp()

const crypto = require('crypto');

exports.report = functions.https.onRequest(async (request, response) => {
  var payload = request.query.payload;
  var provided_hmac = request.query.hmac;
  if (payload === undefined || provided_hmac === undefined) {
    response.status(403).send("Unauthorized");
    return;
  }

  var hmac = crypto.createHmac('sha256', KEY);
  hmac.update(payload);
  if (hmac.digest('hex') != provided_hmac) {
    response.status(403).send("Unauthorized");
    return;
  }

  data = payload.split(',');

  var provided_sequence = parseInt(data[1]);
  if (isNaN(provided_sequence)) {
    response.status(403).send("Unauthorized");
    return;
  }
  
  var sequence = await admin.database().ref('sequence').once('value').then(snapshot => snapshot.val() || 0);

  if (provided_sequence <= sequence) {
    response.status(403).send("Unauthorized");
    return;
  }

  await admin.database().ref('sequence').set(provided_sequence);

  var value = data[0];
  var stall_id = sensor_ids[value & ~0x0f];
  if (stall_id !== undefined) {
    var occupied = (value & 0x0f) == 0b1010;
    await admin.database().ref('stalls/' + stall_id).set({
      'occupied': occupied,
      'timestamp': { '.sv': 'timestamp' }
    });
  }
  response.send("OK");
});
