/**
 * 
 * Plano:
 * 
 *  (0, 0)  (0, 1)  (0, 2)  (0, 3)
 *  (1, 0)  (1, 1)  (1, 2)  (1, 3)
 *  (2, 0)  (2, 1)  (2, 2)  (2, 3)
 *  (3, 0)  (3, 1)  (3, 2)  (3, 3)
 *  
 * Atribuição dos pinos:
 *  
 * Plano 0 -> 13
 * Plano 1 -> 12
 * Plano 2 -> 11
 * Plano 3 -> 10
 * 
 * (0, 0) -> A0
 * (0, 1) -> A1
 * (0, 2) -> A2
 * (0, 3) -> A3
 * 
 * (1, 0) -> A4
 * (1, 1) -> 5
 * (1, 2) -> 6
 * (1, 3) -> 8
 * 
 * (2, 0) -> 7
 * (2, 1) -> 1
 * (2, 2) -> A5
 * (2, 3) -> 0
 * 
 * (3, 0) -> 2
 * (3, 1) -> 3
 * (3, 2) -> 4
 * (3, 3) -> 9
 * 
 */

#include "TimerOne.h"

#define MAX_CUBE 4 // 4 x 4 x 4
#define MAX_COUNTER 4

#define REFRESH_PERIOD 1000
#define PATTERN_PERIOD 200

#define BLOCKED 0
#define TICK_1MS 1

const unsigned char planePin[MAX_CUBE] = {13, 12, 11, 10};

const unsigned char ledPin[MAX_CUBE][MAX_CUBE] = {
  {A0, A1, A2, A3},
  {A4, 5, 6, 8},
  {7, 1, A5, 0},
  {2, 3, 4, 9}
};

unsigned char cube[2][MAX_CUBE][MAX_CUBE][MAX_CUBE];

volatile unsigned char current;
volatile unsigned char flags;

unsigned long counter[MAX_COUNTER];

unsigned char aux;
unsigned char pattern;

void setup() {

  unsigned char i, j;
  
  // Configura pinos
  for (i = 0; i < MAX_CUBE; i++) {
    
    pinMode(planePin[i], OUTPUT);
    digitalWrite(planePin[i], LOW);

    for (j = 0; j < MAX_CUBE; j++) {
      
      pinMode(ledPin[i][j], OUTPUT);
      digitalWrite(ledPin[i][j], LOW);
      
    }
    
  }

  // Configura variáveis
  current = 0;
  flags = 0;
  for (i = 0; i < MAX_COUNTER; i++) {

    counter[i] = 0L;
    
  }
  

  // Configura Timer1 para atualização do cubo
  Timer1.initialize(REFRESH_PERIOD);
  Timer1.attachInterrupt(timerOneIsr);

  pattern = 0;
  counter[1] = 10000L;

}

void loop() {

  if (flags & (1 << TICK_1MS)) {

    flags &= (unsigned char)~(1 << TICK_1MS);

    for (aux = 0; aux < MAX_COUNTER; aux++) {

      if (counter[aux]) {

        counter[aux] = counter[aux] - 1;
        
      }
      
    }
    
  }

  setPattern(pattern);
    
  if (counter[1] == 0) {
    
    counter[1] = 10000L;

    pattern++;
    if (pattern > 2) {

      pattern = 0;
      
    }
    
  }
  
}

/**
 * Define o padrão a ser exibido
 */
void setPattern (unsigned char pattern) {

  static unsigned char currPattern = 0;
  static unsigned char state = 0;

  if (pattern != currPattern) {

    currPattern = pattern;
    state = 0;
    counter[0] = 0;
    
  }

  if (currPattern == 0) {

    state = planesPattern(state, 0, 1);
    
  }
  else if (currPattern == 1) {

    state = planesPattern(state, 1, 1);
    
  }
  else if (currPattern == 2) {

    state = planesPattern(state, 2, 0);
    
  }
  else {

   //state = AllPlanesOnPattern(state);
    
  }
  
}

/**
 * Varre os planos
 */
unsigned char planesPattern (unsigned char state, unsigned char plane, unsigned char dir) {

  static unsigned char curr = 0;
  unsigned char c, x, y, z;
  unsigned char *ptr;

  if (state == 0) {

    c = getNext();

    if (plane == 0) {

      ptr = &x;
      
    }
    else if (plane == 1) {

      ptr = &y;
      
    }
    else {

      ptr = &z;
      
    }

    for (x = 0; x < MAX_CUBE; x++) {

      for (y = 0; y < MAX_CUBE; y++) {

        for (z = 0; z < MAX_CUBE; z++) {

          if (*ptr == curr) {

            cube[c][x][y][z] = HIGH;
            
          }
          else {

            cube[c][x][y][z] = LOW;
            
          }
          
        }
        
      }
      
    }

    setNext(c);

    counter[0] = PATTERN_PERIOD;
    state = 1;
    
  }
  else if (state == 1) {

    if (counter[0] == 0) {

      if (dir == 0) {
        
        curr++;
        
      } else {

        curr--;
        
      }

      curr &= (MAX_CUBE - 1); // Só funciona se MAX_CUBE for uma potência de 2

      state = 0;
      
    }
    
  }
  else {

    curr = 0;
    counter[0] = 0;
    state = 0;
    
  }

  return state;
  
}

/**
 * Pega o próximo cubo a ser utilizado no refresh
 */
unsigned char getNext() {

  if (current == 0) {
    
    return 1;
    
  }
  
  return 0;
  
}

/**
 * Define o próximo cubo a ser utilizado no refresh
 */
void setNext (unsigned char next) {

  while (flags & (1 << BLOCKED));

  current = next;
  
}

/**
 * Interrupção do Timer1 atualiza imagem do cubo
 */
void timerOneIsr() {

  static unsigned char state = 0;
  static unsigned char i = 0;
  unsigned char j, k;

  flags |= (1 << TICK_1MS);
  
  if (state == 0) {
    
    // Bloqueia atualização exterior da figura
    flags |= (1 << BLOCKED);
  
    // Atualiza novo plano
    for (j = 0; j < MAX_CUBE; j++) {
  
      for (k = 0; k < MAX_CUBE; k++) {
  
        digitalWrite(ledPin[j][k], cube[current][i][j][k]);
        
      }
      
    }
  
    // Mostra novo plano
    digitalWrite(planePin[i], HIGH);

    // Desbloqueia atualização exterior da figura
    flags &= (unsigned char)~(1 << BLOCKED);

    state = 1;
    
  }
  else {
    
    // Apaga plano anterior
    digitalWrite(planePin[i], LOW);

    // Vai para o próximo plano
    i = (i + 1) & (MAX_CUBE - 1);

    state = 0;
    
  }
  
}

