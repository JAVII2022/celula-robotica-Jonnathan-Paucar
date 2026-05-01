// DEFINICIÓN DE PINES 
#define LED_IDLE 2          // LED Estado 1 - IDLE (azul)
#define LED_AUTH 4          // LED Estado 2 - AUTH (verde)
#define LED_CLASS 5         // LED Estado 3 - CLASS (amarillo)
#define LED_PROCESS 18      // LED Estado 4 - PROCESS (rojo - brazo robótico)
#define LED_INSPECT 19      // LED Estado 5 - INSPECT (azul2 - visión artificial)
#define LED_READY 21        // LED Estado 6 - READY (verde2 - entrega)      

#define BOTON_INICIO 15
#define BOTON_DETECCION 16
#define BOTON_CLASIFICACION 17
#define BOTON_REINICIO 22

// VARIABLES GLOBALES
volatile int state = 1;
volatile int contador = 0;
volatile int tiempoProceso = 0;
volatile int procesoActivo = 0;

// Flags para sincronización
volatile int flagInicio = 0;
volatile int flagDeteccion = 0;
volatile int flagClasificacion = 0;
volatile int flagReinicio = 0;
volatile int flagTimer = 0;

// Timer hardware
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// PROTOTIPOS
void IRAM_ATTR onTimer();
void IRAM_ATTR isrInicio();
void IRAM_ATTR isrDeteccion();
void IRAM_ATTR isrClasificacion();
void IRAM_ATTR isrReinicio();
void apagarTodosLosLEDs();

void setup() 
{
  Serial.begin(9600);
  Serial.println("Célula de Manufactura Robótica Autónoma");
  
  // Configurar pines de LEDs
  pinMode(LED_IDLE, OUTPUT);
  pinMode(LED_AUTH, OUTPUT);
  pinMode(LED_CLASS, OUTPUT);
  pinMode(LED_PROCESS, OUTPUT);
  pinMode(LED_INSPECT, OUTPUT);
  pinMode(LED_READY, OUTPUT);
  
  // Apagar todos los LEDs
  apagarTodosLosLEDs();
  
  // Configurar pines de botones
  pinMode(BOTON_INICIO, INPUT_PULLUP);
  pinMode(BOTON_DETECCION, INPUT_PULLUP);
  pinMode(BOTON_CLASIFICACION, INPUT_PULLUP);
  pinMode(BOTON_REINICIO, INPUT_PULLUP);
  
  // Configurar interrupciones externas (FALLING = cuando se presiona)
  attachInterrupt(digitalPinToInterrupt(BOTON_INICIO), isrInicio, FALLING);
  attachInterrupt(digitalPinToInterrupt(BOTON_DETECCION), isrDeteccion, FALLING);
  attachInterrupt(digitalPinToInterrupt(BOTON_CLASIFICACION), isrClasificacion, FALLING);
  attachInterrupt(digitalPinToInterrupt(BOTON_REINICIO), isrReinicio, FALLING);
  
  // Configurar Timer Hardware (1 segundo)
  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000000, true, 0);
  
  digitalWrite(LED_IDLE, HIGH);
  Serial.println("Sistema Iniciado - Estado 1: IDLE (Reposo)");
  Serial.println("Presione BOTON 1 para comenzar");
}

void loop() 
{
  // MANEJAR REINICIO - AHORA FUNCIONA EN CUALQUIER ESTADO
  if(flagReinicio == 1) 
  {
    flagReinicio = 0;
    
    // Detener cualquier proceso activo
    procesoActivo = 0;
    tiempoProceso = 0;
    
    // Reiniciar al estado IDLE
    state = 1;
    apagarTodosLosLEDs();
    digitalWrite(LED_IDLE, HIGH);
    
    Serial.println("\n>>> SISTEMA REINICIADO POR USUARIO <<<");
    Serial.println("Estado 1: IDLE (Reposo)");
    Serial.println("Presione BOTON 1 para comenzar\n");
  }
  
  // Manejar interrupción de inicio
  if(flagInicio == 1 && state == 1) 
  {
    flagInicio = 0;
    state = 2;
    apagarTodosLosLEDs();
    digitalWrite(LED_AUTH, HIGH);
    Serial.println(">>> Estado 2: AUTH - Pieza detectada");
    Serial.println("Presione BOTON 2 para clasificar");
  }
  
  // Manejar interrupción de detección
  if(flagDeteccion == 1 && state == 2) 
  {
    flagDeteccion = 0;
    state = 3;
    apagarTodosLosLEDs();
    digitalWrite(LED_CLASS, HIGH);
    Serial.println(">>> Estado 3: CLASS - Clasificación por IA");
    Serial.println("Presione BOTON 3 para ensamblar");
  }
  
  // Manejar interrupción de clasificación
  if(flagClasificacion == 1 && state == 3) 
  {
    flagClasificacion = 0;
    state = 4;
    apagarTodosLosLEDs();
    digitalWrite(LED_PROCESS, HIGH);
    procesoActivo = 1;
    tiempoProceso = 0;
    Serial.println(">>> Estado 4: PROCESS - Brazo robótico ensamblando (5 segundos)");
  }
  
  // Manejar flags del timer para procesos
  if(flagTimer == 1) 
  {
    flagTimer = 0;
    
    if(procesoActivo == 1 && state == 4) 
    {
      tiempoProceso++;
      Serial.print("PROCESS: ");
      Serial.print(tiempoProceso);
      Serial.println(" segundos...");
      
      if(tiempoProceso >= 5) 
      {
        state = 5;
        procesoActivo = 2;
        tiempoProceso = 0;
        apagarTodosLosLEDs();
        digitalWrite(LED_INSPECT, HIGH);
        Serial.println("\n>>> Estado 5: INSPECT - Inspección de calidad (3 segundos)");
      }
    }
    else if(procesoActivo == 2 && state == 5) 
    {
      tiempoProceso++;
      Serial.print("INSPECT: ");
      Serial.print(tiempoProceso);
      Serial.println(" segundos...");
      
      if(tiempoProceso >= 3) 
      {
        state = 6;
        procesoActivo = 0;
        tiempoProceso = 0;
        apagarTodosLosLEDs();
        digitalWrite(LED_READY, HIGH);
        Serial.println("\n>>> Estado 6: READY - Pieza lista para entrega");
        Serial.println("Presione BOTON 4 para reiniciar el sistema");
      }
    }
  }
  
  // Mantener LED del estado actual encendido
  if(state == 1) digitalWrite(LED_IDLE, HIGH);
  else if(state == 2) digitalWrite(LED_AUTH, HIGH);
  else if(state == 3) digitalWrite(LED_CLASS, HIGH);
  else if(state == 4) digitalWrite(LED_PROCESS, HIGH);
  else if(state == 5) digitalWrite(LED_INSPECT, HIGH);
  else if(state == 6) digitalWrite(LED_READY, HIGH);
}

// ISR DEL TIMER 
void IRAM_ATTR onTimer() 
{
  portENTER_CRITICAL_ISR(&timerMux);
  flagTimer = 1;
  portEXIT_CRITICAL_ISR(&timerMux);
}

// INTERRUPCIONES EXTERNAS
void IRAM_ATTR isrInicio() 
{
  flagInicio = 1;
}

void IRAM_ATTR isrDeteccion() 
{
  flagDeteccion = 1;
}

void IRAM_ATTR isrClasificacion() 
{
  flagClasificacion = 1;
}

void IRAM_ATTR isrReinicio() 
{
  flagReinicio = 1;
}

//FUNCIÓN APAGAR LEDS
void apagarTodosLosLEDs() 
{
  digitalWrite(LED_IDLE, LOW);
  digitalWrite(LED_AUTH, LOW);
  digitalWrite(LED_CLASS, LOW);
  digitalWrite(LED_PROCESS, LOW);
  digitalWrite(LED_INSPECT, LOW);
  digitalWrite(LED_READY, LOW);
}