const int TRIG_PIN = 5;
const int ECHO_PIN = 18;

const bool FORMAT_JSON = false;   // false : ligne lisible, true : JSON (pour l'envoi)

const float H_REF_CM = 40.0;
const float D_MIN_CM = 2.0;
const float TOLERANCE_CM = 2.0;
const int   N_MESURES = 9;
const int   N_MIN_VALIDES = 5;
const float SEUIL_VIGILANCE = 50.0;
const float SEUIL_ALERTE    = 80.0;

float lireDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duree = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duree == 0) return -1;
  return duree * 0.0343 / 2.0;
}

float mediane(float *a, int n) {
  for (int i = 1; i < n; i++) {
    float v = a[i]; int j = i - 1;
    while (j >= 0 && a[j] > v) { a[j + 1] = a[j]; j--; }
    a[j + 1] = v;
  }
  return (n % 2) ? a[n / 2] : (a[n / 2 - 1] + a[n / 2]) / 2.0;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  float m[N_MESURES];
  int valides = 0, trop_proche = 0, trop_loin = 0, sans_echo = 0;

  for (int i = 0; i < N_MESURES; i++) {
    float d = lireDistanceCm();
    if (d < 0)                            sans_echo++;
    else if (d < D_MIN_CM)                trop_proche++;
    else if (d > H_REF_CM + TOLERANCE_CM) trop_loin++;
    else                                  m[valides++] = d;
    delay(60);
  }

  if (valides < N_MIN_VALIDES) {
    const char *cause;
    if (sans_echo >= trop_proche && sans_echo >= trop_loin)
      cause = "aucun echo recu (capteur debranche ou rien en face)";
    else if (trop_proche >= trop_loin)
      cause = "objet a moins de 2 cm du capteur";
    else
      cause = "rien detecte a moins de 42 cm";

    if (FORMAT_JSON)
      Serial.printf("{\"etat\":\"indetermine\",\"valides\":%d,\"sur\":%d}\n", valides, N_MESURES);
    else
      Serial.printf("MESURE IMPOSSIBLE : %s\n", cause);

  } else {
    float dist = mediane(m, valides);
    float niveau = H_REF_CM - dist;
    if (niveau < 0) niveau = 0;
    float pct = niveau / (H_REF_CM - D_MIN_CM) * 100.0f;
    if (pct > 100) pct = 100;
    float niveau_alerte = SEUIL_ALERTE / 100.0f * (H_REF_CM - D_MIN_CM);
    float marge = niveau_alerte - niveau;
    if (marge < 0) marge = 0;

    const char *etat = "NORMAL";
    if (pct >= SEUIL_ALERTE) etat = "ALERTE";
    else if (pct >= SEUIL_VIGILANCE) etat = "VIGILANCE";

    if (FORMAT_JSON) {
      Serial.printf("{\"distance_cm\":%.1f,\"niveau_pct\":%.0f,\"etat\":\"%s\",\"marge_alerte_cm\":%.1f,\"valides\":%d,\"sur\":%d}\n",
                    dist, pct, etat, marge, valides, N_MESURES);
    } else {
      char barre[11];
      int blocs = (int)(pct / 10.0f + 0.5f);
      for (int i = 0; i < 10; i++) barre[i] = (i < blocs) ? '#' : '.';
      barre[10] = 0;

      Serial.printf("%.1f cm | [%s] %.0f%% | %s", dist, barre, pct, etat);
      if (marge > 0) Serial.printf(" | alerte dans %.1f cm", marge);
      if (valides < N_MESURES) Serial.printf(" | fiabilite %d/%d", valides, N_MESURES);
      Serial.println();
    }
  }
  delay(1000);
}