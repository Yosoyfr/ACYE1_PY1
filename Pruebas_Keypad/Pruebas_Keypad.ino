#include <Keypad.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

int pos = 0; // LCD Connections
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
const byte rows = 4;
const byte cols = 3;
char key[rows][cols] = {
  {'7', '8', '9'},
  {'4', '5', '6'},
  {'1', '2', '3'},
  {'*', '0', 'E'}
};
byte rowPins[rows] = {5, 4, 3, 2};
byte colPins[cols] = {8, 7, 6};
Keypad keypad = Keypad(makeKeymap(key), rowPins, colPins, rows, cols);
char* password[8]; //Esta variable será util para ver si el jefe autoriza la operación.
char* bPassword = "0106";
char uss[4] = {'\0', '\0', '\0', '\0'};
char pass[8] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
int currentposition = 0;
int invalidcount = 12;
bool passInput = false, usrInput = true;

/*
   Struct que representa un usuario en el sistema
*/
typedef struct  {
  int id;
  long password;
} User;

void setup()
{
  Serial.begin(9600);
  //Configuracion de la pantalla LCD
  displayscreen();
  lcd.begin(16, 2);
  //Configuracion de un boton reset system
  pinMode(13, INPUT);
  showUsers();
}

void loop()
{
  if ( currentposition == 0)
  {
    displayscreen();
  }
  int l ;
  char code = keypad.getKey();
  if (code != NO_KEY)
  {
    if (code != 'E' )
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("USUARIO:");
      lcd.setCursor(7, 1);
      lcd.print(" ");
      lcd.setCursor(7, 1);
      uss[currentposition] = code;
      for (l = 0; l <= currentposition; ++l)
      {
        lcd.print(uss[l]);
      }
      ++currentposition;
    }
    else
    {
      // Área para verificar si pertenece a la matriz.
      if (strncmp(uss, "0000", 4 ) == 0)
      {
        addNewUser();
      }
      else if (verificarPass())
      {
        correct();
        invalidcount = 0;
      }
      else
      {
        ++invalidcount;
        incorrect();
      }
      currentposition = 0;
      memset (uss, '\0', 4);
      memset (pass, '\0', 8);
    }
  }
  //Resetea el sistema de usuarios completamente
  if (digitalRead(13)) {
    Serial.println("Restaurando el sistema");
    clearEEPROM();
  }
}

void clearEEPROM()
{
  Serial.println("EEPROM");
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    if (EEPROM.read(i) != 0)                    //skip already "empty" addresses
    {
      EEPROM.write(i, 0);                       //write 0 to address i
    }
  }
  Serial.println("EEPROM erased");                            //reset address counter
}

void incorrect()
{
  memset(pass, '\0', 4);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("PASS");
  lcd.setCursor(6, 0);
  lcd.print("INCORRECTO");
  lcd.setCursor(15, 1);
  lcd.println(" ");
  delay(2000);
  lcd.clear();
  displayscreen();
}

void correct()
{
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("PASS");
  lcd.setCursor(6, 0);
  lcd.print("CORRECTO");
  lcd.setCursor(15, 1);
  lcd.println(" ");
  delay(2000);
  lcd.clear();
  displayscreen();
}

/*
   Metodo que recupera el numero de usuarios registrados en el sistema
   de la memeria EEPROM del arduino, la cual serian las primeras dos posiciones
*/
int contadorUsuarios() {
  int nUsuarios;
  EEPROM.get(0, nUsuarios);
  return nUsuarios;
}

/*
   Metodo para insertar un nuevo usuario del sistema a la memoria EEPROM
*/
void addUser(User usuario) {
  //Ingresamos el nuevo usuario
  int nUsuarios = contadorUsuarios();
  int index = nUsuarios * sizeof(User) + 2;
  EEPROM.put(index, usuario);
  //Aumentamos el contador de usuarios
  nUsuarios++;
  EEPROM.put(0, nUsuarios);
}

void displayscreen()
{
  lcd.setCursor(0, 0);
  lcd.println("*BIENVENIDO*");
}

bool verificarPass() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println("PASSWORD:");
  lcd.setCursor(0, 1);
  int passC = 0;
  while (true)
  {
    char code = keypad.getKey();
    if (code != NO_KEY)
    {
      if (code != 'E')
      {
        pass[passC++] = code;
        lcd.print(code);
      }
      else if (passC >= 8)
      {
        break;
      }
      else
      {
        break;
      }
    }
  }
  delay(2000);  
  return usersLogin(String(uss).toInt(), atol(pass));
}

/*
   Metodo para verificar el log correcto de un usuario
*/
bool usersLogin(int id, long password) {
  int address = 2;
  for (int i = 0 ; i < contadorUsuarios() ; ++i) {
    User user;
    EEPROM.get(address, user);
    if (user.id == id && user.password == password) {
      Serial.println("Usuario encontrado");
      return true;
    }
    address += sizeof(User);
  }
  Serial.println("Usuario no encontrado");
  return false;
}

/*
   Metodo para mostrar todos los usuarios en el sistema guardados en la EEPROM
*/
void showUsers() {
  int address = 2;
  for (int i = 0 ; i < contadorUsuarios() ; ++i) {
    User user;
    EEPROM.get(address, user);
    Serial.print("ID: ");
    Serial.print(user.id);
    Serial.print(" Password: ");
    Serial.println(user.password);
    address += sizeof(User);
  }
}

void addNewUser()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PASS:");
  lcd.setCursor(0, 1);
  char newPass[8] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
  char newConfirm[8] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
  int count = 0;
  while (true)
  {
    char code = keypad.getKey();
    if (code != NO_KEY)
    {
      if (code == 'E')
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("CONFIRMAR:");
        lcd.setCursor(0, 1);

        int confirmCount = 0;
        while (confirmCount < 8)
        {
          code = keypad.getKey();
          if (code != NO_KEY)
          {
            if (code == 'E')break;
            newConfirm[confirmCount] = code;
            lcd.print(code);
            confirmCount++;
          }
        }
        delay(1000);
        bool match = (strncmp(newPass, newConfirm, 8) == 0);
        if (match)
        {
          //Pedir autorización del gerente.
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("GERENTE:");
          lcd.setCursor(0, 1);
          confirmCount = 0;
          char bossPass[4];
          while (confirmCount < 4)
          {
            code = keypad.getKey();
            if (code != NO_KEY)
            {
              bossPass[confirmCount] = code;
              lcd.print(code);
              confirmCount++;
            }
          }
          bool autorized = (strncmp(bossPass, bPassword, 4) == 0);

          if (autorized)
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("AUTORIZADA");
            delay(2000);

            //Imprimir nuevo codigo de usuario
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("USUARIO:");
            lcd.setCursor(0, 1);
            int newCode = contadorUsuarios();
            lcd.println(String(newCode));
            delay(2000);

            //Imprimir su contraseña
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("PASS:");
            lcd.setCursor(0, 1);
            lcd.println(String(newPass));
            delay(2000);

            //Agregarlo a la EEPROM
            User newUser = {(int)newCode, atol(newPass)};
            addUser(newUser);
          }
          else
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("NO AUTORIZADA");
            delay(2000);
          }
        }
        else
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("PASS NO COINCIDE");
          delay(2000);
        }
        //Romper el ciclo
        break;
      }
      else if (count >= 8)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PASS MAX DE 8");
        delay(2000);
        break;
      }
      else
      {
        //Almacenar en newPass el caracter deseado.
        newPass[count] = code;
        count++;
        //Imprimir la contraseña.
        lcd.print(code);
      }
    }
  }
  lcd.clear();
}
