bool ler(int a) {
  return digitalRead(a);
}
bool porta() {
  bool a = digitalRead(sensorporta);
  return a;
}
bool parar() {
  if (porta() == 0 && ler(sensortempmax) == 0) {
    return 0;
  } else {
    return 1;
  }
}
bool encher() {
  if (parar() == 0 && ler(sensornvagua) == 0) {
    digitalWrite(enchimento, 1);
    return 0;
  } else if (parar() == 0 && ler(sensornvagua) == 1) {
    digitalWrite(enchimento, 0);
    return 1;
  }
}
int temp() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempC = sensors.getTempCByIndex(0);
  int temp = int(tempC);
  // Check if reading was successful
  if (tempC != DEVICE_DISCONNECTED_C) {
    return temp;
  } else {
    return 50;
  }
}
bool fluxoagua() {
  const unsigned long tempoprelavar = 360000;
  static unsigned long tempoanterior = millis();
  if (parar() == 0 && encher() == 1) {
    if (millis() - tempoanterior >= tempoprelavar) {
      digitalWrite(motorfluxoagua, 1);
      return 1;
    }
  } else {
    digitalWrite(motorfluxoagua, 0);
    return 0;
  }
}
void prelavar() {
  const unsigned long tempoprelavar = 360000;
  static unsigned long tempoanterior = millis();
  //static unsigned long pausa = tempoanterior;
  bool fim = 0;
  if (encher() == 1 && parar == 0) {
    if (fim == 0 && digitalRead(motorfluxoagua) == 0) {
      digitalWrite(motorfluxoagua, 1);
      tempoanterior = millis();
    }
    if (porta() == 1) {
      tempoanterior = millis() - tempoanterior;
    } else {
      if (millis() - tempoanterior >= tempoprelavar) {
        fim = 1;
        digitalWrite(motorfluxoagua, 0);
      }
    }
  }
}
void prelavar2() {
  const unsigned long tempoprelavar = 360000;
  static unsigned long tempoanterior = millis();
  const unsigned long tempojogarfora = 10000;
  static unsigned long tempoanteriorjogarfora = millis();
  static bool cheio = 0;
  static bool fimfluxo = 0;
  static bool jogandofora = 0;
  if (cheio == 0) {
    if (encher() == 0 && cheio == 0) {
      digitalWrite(motorfluxoagua, 1);
      cheio = 1;
      tempoanterior = millis();
    } else if (encher() == 1 && cheio == 1 && fimfluxo == 0 && porta() == 0) {
      digitalWrite(motorfluxoagua, 1);
    }
  }
  else if (cheio == 1 && fimfluxo == 0 && porta() == 0) {
    if (porta() == 0 && millis() - tempoanterior >= tempoprelavar) {
      digitalWrite(motorfluxoagua, 0);
      fimfluxo = 1;
      tempoanteriorjogarfora = millis();
    }
    else if (porta() == 1 && cheio == 1 && fimfluxo == 0) {
      tempoanterior = millis() - tempoanterior;
    }
  }
  else if (cheio == 1 && fimfluxo == 1 && jogandofora == 0 && encher() == 1) {
    digitalWrite(bomba, 1);
    if (millis() - tempoanteriorjogarfora >= tempojogarfora) {
      digitalWrite(bomba, 0);
      jogandofora = 1;
    }
  }
}
void 
