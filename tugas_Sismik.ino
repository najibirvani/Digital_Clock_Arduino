int segment[] ={7,6,11,12,13,8,9,10};     // pin a, b, c, d, e, f, g, dp
int push[]    ={A2,A1,A0,A3};             // push[0]=menu, push[1]=select, push[2]=plus, push[3]=minus
int ledPin[]  ={A4,A5};                   // LED Pin detik
int buzzer    = 1;
int seg_con[]  = {5,4,3,2};               // Selector 7-segment
int halfSecond = 0;                       // Nilai tiap 500ms
int loopAlarm;

// Boolean untuk aktifasi setiap event
bool isProgramRun = false;
bool select = false;
bool startStopwatch = false;
bool showFinalStopwatch = true;
bool startTimer = true;
bool startJam = true;
bool flag = true;
bool isStopwatch;
bool doneTimer = false;

// Variabel pengaturan mode dan timer
unsigned char mode = 1;
unsigned long int timer;
unsigned long int lasttimer=0;

// Konstanta waktu
signed char jam_satuan, jam_puluhan, menit_satuan, menit_puluhan, detik_puluhan, detik_satuan;
signed char timer_menitPuluhan, timer_menitSatuan, timer_detikPuluhan, timer_detikSatuan;

// Set up Program
void setup() 
{  
  // Pengesetan pin output untuk 7-segmen
  for (int i=0; i<8; i++) pinMode (segment[i],OUTPUT);
  
  for (int i=0;i<4;i++){
     pinMode (push[i], INPUT);        // Pengesetan input push button
     pinMode (seg_con[i], OUTPUT);    // pengesetan output control pada 7-segment
     //digitalWrite(push[i],HIGH);      // pengesetan nilai control 7-segmen
  }
  pinMode (ledPin[0],OUTPUT);         // Pengesetan LED
  pinMode (ledPin[1],OUTPUT);         // Pengesetan LED 
  pinMode (buzzer,OUTPUT);         // Pengesetan Buzzer  

  // Stop interrupts
  cli();
  
  // Initialize Timer/Counter Register
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; //initialize counter value to 0

  // Turn on CTC mode
  TCCR1B |= (1 << WGM12);
  
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  
  // a value of 15624 in a second. By having OCR1A = 15624 / 2 then the interrupt function
  OCR1A = 15624 / 2;
  
  // Enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  // Allow interrupts
  sei();

}

// Prosedur Led On untuk jam digital
void ledOn()
{
  digitalWrite(ledPin[0],HIGH);
  digitalWrite(ledPin[1],HIGH);
}

// Prosedur Led On untuk jam digital
void ledOff()
{
  digitalWrite(ledPin[0],LOW);
  digitalWrite(ledPin[1],LOW);
}

// jam dengan interrupt
ISR(TIMER1_COMPA_vect)
{
  halfSecond++; 
  
  if (halfSecond==1) // Untuk kedipan lampu indikator detik
  {
    // Untuk mode jam digital led berkedip setiap detik
    if (mode==1 && isProgramRun)
    {
      ledOff(); 
    }
  }else if (halfSecond!=1)
  {
    halfSecond=0;
    detik_satuan++;
    if (mode==1 && isProgramRun)
    {
      ledOn(); 
    }
    else if (mode==3 && isProgramRun)
    {
      timer_detikSatuan--; 
    }
  }

  if (mode==1 || mode==2)
  {
      if (detik_satuan > 9)
      {
        detik_satuan = 0;
        detik_puluhan++;
        
        // nilai detik sudah mencapai 60
        if(detik_puluhan > 5 ) 
        {               
          detik_puluhan = 0;
          menit_satuan++;
           
          if(menit_satuan > 9)
          {
            menit_satuan=0;
            menit_puluhan++;
        
          // nilai menit sudah mencapai 60
            if (menit_puluhan > 5)
            {
              menit_puluhan=0;
              jam_satuan++;
            
              if(jam_satuan > 9)
              {
                jam_satuan=0;
                jam_puluhan++;
        
                // nilai jam mecapai 24
                if(jam_puluhan == 2 && jam_satuan== 4)
                {
                  jam_puluhan=0;
                  jam_satuan=0;
                }
              }
            }
          }
        }    
      } 
  }
  // Mode timer konstanta berkurang
  if (mode==3)
  {
      if (timer_detikSatuan<0)
      {
        timer_detikSatuan = 9;
        timer_detikPuluhan--;

        if (timer_detikPuluhan<0)
        {
          timer_detikPuluhan = 5;
          timer_menitSatuan--;

          if (timer_menitSatuan<0)
          {
            timer_menitSatuan = 9;
            timer_menitPuluhan--;

            if (timer_menitPuluhan<0)
            {
              timer_menitPuluhan = 0;
              timer_menitSatuan = 0;
              timer_detikPuluhan = 0;
              timer_detikSatuan = 0;
              isProgramRun = false;
              doneTimer = true;
              alarmON();
            }
          }
        }

        
      }
  }
}

// Inisiasi nilai kontanta sama dengan nol
void init_angka()
{
  //inisialisasi nilai detik, menit, jam
  detik_satuan=0; detik_puluhan=0; 
  menit_satuan=0; menit_puluhan=0;
  jam_satuan=0; jam_puluhan=0;
}

//pengaturan menggunakan push button
void setting_angka ()
{
  unsigned char x = 5; 
  signed char digit1, digit2, digit3, digit4;
  unsigned char maksimumDigit1, maksimumDigit2;

  isProgramRun = false;
  
  switch (mode)
  {
    case 1 : {   // Jam Digital
              digit1  = jam_puluhan;
              digit2  = jam_satuan;
              digit3  = menit_puluhan;
              digit4  = menit_satuan;
              maksimumDigit1 = 2;
              maksimumDigit2 = 3;
             }
             break;
    case 3 : {   // Timer
              digit1  = menit_puluhan;
              digit2  = menit_satuan;
              digit3  = detik_puluhan;
              digit4  = detik_satuan;
              maksimumDigit1 = 5;
              maksimumDigit2 = 9;
             }
             break;
  }

  // Pembacaan tombol mode
  if(digitalRead(push[1])==0)
  {
      delay(200);
      x=4;
  }
    
  // setting menit satuan atau detik satuan
  while(x==4) 
  {   
      if(digitalRead(push[1])==0) // Tombol select untuk geser digit
      {
          delay(200);
          x=3;
      }
  
      if (digitalRead(push[2])==0) // Tombol plus
      {
          delay(200);
          digit4++;
          
          if(digit4>9) 
            digit4 = 0;
      }
      
      if(digitalRead(push[3])==0) // Tombol minus
      {
          delay(200);
          digit4--;
          
          if(digit4<0) 
            digit4=9;
      }

      // Menampilkan digit ke empat
      digitalWrite(seg_con[3],HIGH);
      digitalWrite(seg_con[2],LOW);
      digitalWrite(seg_con[1],LOW);
      digitalWrite(seg_con[0],LOW);
      tampil_7(digit4);
      
  }
    
    // setting menit puluhan
  while(x==3)
  {  
      if(digitalRead(push[1])==0) // Tombol select untuk geser digit
      {
          delay(200);
          x=2;
      }
      
      if (digitalRead(push[2])==0) // Tombol plus
      {
          delay(200);
          digit3++;
          
          if(digit3>5) 
            digit3=0;
      }
      
      if(digitalRead(push[3])==0) // Tombol minus
      {
          delay(200);
          digit3--;
          
          if(digit3<0) 
            digit3=5;
          
      }

      // Menampilkan digit ketiga
      digitalWrite(seg_con[3],LOW);
      digitalWrite(seg_con[2],HIGH);
      digitalWrite(seg_con[1],LOW);
      digitalWrite(seg_con[0],LOW);
      tampil_7(digit3);
  } 
     
  // setting jam satuan
  while(x==2)
  {     
    if(digitalRead(push[1])== 0) // Tombol select untuk geser digit
    {
      delay(200);
      x=1;
    }
  
    if (digitalRead(push[2])==0) // Tombol plus
    {
      delay(200);
      digit2++;
      
      if(digit2 > maksimumDigit2) 
      {
        digit2 = 0;
      }
    }
    
    if(digitalRead(push[3])==0) // Tombol minus
    {
        delay(200);
        digit2--;
        
        if(digit2 < 0)
        {
          digit2 = maksimumDigit2; 
        }    
    }

    // Menampilkan digit kedua
    digitalWrite(seg_con[3],LOW);
    digitalWrite(seg_con[2],LOW);
    digitalWrite(seg_con[1],HIGH);
    digitalWrite(seg_con[0],LOW);
    tampil_7(digit2);
  } 
      
  // setting jam puluhan  
  while(x==1)
  {     
    if(digitalRead(push[1])==0) // Tombol select untuk geser digit
    {
        delay(200);
        x=5;
        //detik_satuan=0;
        //detik_puluhan=0;
    }
  
    if (digitalRead(push[2])==0) // Tombol plus
    {
        delay(200);
        digit1++;
        
        if(digit1 > maksimumDigit1)
        {
          digit1 = 0;  
        }
    }
    
    if(digitalRead(push[3])==0) // Tombol minus
    {
        delay(200);
        digit1--;
        
        if(digit1 < 0) 
          digit1 = maksimumDigit1;          
    }
    
    // Menampilkan digit kesatu
    digitalWrite(seg_con[3],LOW);
    digitalWrite(seg_con[2],LOW);
    digitalWrite(seg_con[1],LOW);
    digitalWrite(seg_con[0],HIGH);
    tampil_7(digit1);
  }

  // Mengisi nilai konstanta waktu dengan hasil setting oleh user
  if (mode==1)
  {
    jam_puluhan = digit1;
    jam_satuan = digit2;
    menit_puluhan = digit3;
    menit_satuan = digit4;
    detik_puluhan = 0;
    detik_satuan = 0;
    halfSecond = 0;
  } else if (mode==3)
  {
    timer_menitPuluhan = digit1;
    timer_menitSatuan = digit2;
    timer_detikPuluhan = digit3;
    timer_detikSatuan = digit4;
    halfSecond = 0;
  }
  
  halfSecond = 0;
}

// Menampilakn angka pada 7-segment
void tampil_7(int nilai) 
{
  if (nilai==0){
            digitalWrite(segment[0], LOW);
            digitalWrite(segment[1], LOW);
            digitalWrite(segment[2], LOW);
            digitalWrite(segment[3], LOW);
            digitalWrite(segment[4], LOW);      
            digitalWrite(segment[5], LOW);
            digitalWrite(segment[6], HIGH);
            digitalWrite(segment[7], HIGH);
  }
  else if (nilai==1) {
            digitalWrite(segment[0], HIGH);
            digitalWrite(segment[1], LOW);
            digitalWrite(segment[2], LOW);
            digitalWrite(segment[3], HIGH);
            digitalWrite(segment[4], HIGH);
            digitalWrite(segment[5], HIGH);
            digitalWrite(segment[6], HIGH);
            digitalWrite(segment[7], HIGH);   
  }
  else if (nilai==2){
            digitalWrite(segment[0], LOW);
            digitalWrite(segment[1], LOW);
            digitalWrite(segment[2], HIGH);
            digitalWrite(segment[3], LOW);
            digitalWrite(segment[4], LOW);
            digitalWrite(segment[5], HIGH);
            digitalWrite(segment[6], LOW);
            digitalWrite(segment[7], HIGH);
  }
  else if (nilai==3)  {
          digitalWrite(segment[0], LOW);
          digitalWrite(segment[1], LOW);
          digitalWrite(segment[2], LOW);
          digitalWrite(segment[3], LOW);
          digitalWrite(segment[4], HIGH);
          digitalWrite(segment[5], HIGH);
          digitalWrite(segment[6], LOW);
          digitalWrite(segment[7], HIGH);
  }
  else if (nilai==4){
            digitalWrite(segment[0], HIGH);
            digitalWrite(segment[1], LOW);
            digitalWrite(segment[2], LOW);
            digitalWrite(segment[3], HIGH);
            digitalWrite(segment[4], HIGH);
            digitalWrite(segment[5], LOW);
            digitalWrite(segment[6], LOW);
            digitalWrite(segment[7], HIGH);
  }
  else if (nilai==5){
            digitalWrite(segment[0], LOW);
            digitalWrite(segment[1], HIGH);
            digitalWrite(segment[2], LOW);
            digitalWrite(segment[3], LOW);
            digitalWrite(segment[4], HIGH);
            digitalWrite(segment[5], LOW);
            digitalWrite(segment[6], LOW);
            digitalWrite(segment[7], HIGH);
  }
  else if (nilai==6){
            digitalWrite(segment[0], LOW);
            digitalWrite(segment[1], HIGH);
            digitalWrite(segment[2], LOW);
            digitalWrite(segment[3], LOW);
            digitalWrite(segment[4], LOW);
            digitalWrite(segment[5], LOW);
            digitalWrite(segment[6], LOW);
            digitalWrite(segment[7], HIGH);
  }
  else if (nilai==7){
            digitalWrite(segment[0], LOW);
            digitalWrite(segment[1], LOW);
            digitalWrite(segment[2], LOW);
            digitalWrite(segment[3], HIGH);
            digitalWrite(segment[4], HIGH);
            digitalWrite(segment[5], HIGH);
            digitalWrite(segment[6], HIGH);
            digitalWrite(segment[7], HIGH);
  }
  else if (nilai==8){
            digitalWrite(segment[0], LOW);
            digitalWrite(segment[1], LOW);
            digitalWrite(segment[2], LOW);
            digitalWrite(segment[3], LOW);
            digitalWrite(segment[4], LOW);
            digitalWrite(segment[5], LOW);
            digitalWrite(segment[6], LOW);
            digitalWrite(segment[7], HIGH);
  }
  else if (nilai==9)
  {
            digitalWrite(segment[0], LOW);
            digitalWrite(segment[1], LOW);
            digitalWrite(segment[2], LOW);
            digitalWrite(segment[3], LOW);
            digitalWrite(segment[4], HIGH);
            digitalWrite(segment[5], LOW);
            digitalWrite(segment[6], LOW);
            digitalWrite(segment[7], HIGH);
  }
  else if (nilai==10)
  {
            digitalWrite(segment[0], HIGH);
            digitalWrite(segment[1], HIGH);
            digitalWrite(segment[2], HIGH);
            digitalWrite(segment[3], HIGH);
            digitalWrite(segment[4], HIGH);
            digitalWrite(segment[5], HIGH);
            digitalWrite(segment[6], LOW);
            digitalWrite(segment[7], HIGH);
  }
}

// menampilkan jam digital
void show(signed char a, signed char b, signed char c, signed char d)
{
    digitalWrite(seg_con[3],HIGH);  // Digit Keempat
    digitalWrite(seg_con[2],LOW);
    digitalWrite(seg_con[1],LOW);
    digitalWrite(seg_con[0],LOW);
    tampil_7(d);
    delay(5);
    
    digitalWrite(seg_con[3],LOW);
    digitalWrite(seg_con[2],HIGH);  // Digit Ketiga
    digitalWrite(seg_con[1],LOW);
    digitalWrite(seg_con[0],LOW);
    tampil_7(c);
    delay(5);
   
    digitalWrite(seg_con[3],LOW);   // Digit Kedua
    digitalWrite(seg_con[2],LOW);
    digitalWrite(seg_con[1],HIGH);
    digitalWrite(seg_con[0],LOW);
    tampil_7(b);
    delay(5);
    
    digitalWrite(seg_con[3],LOW);   // Digit Pertama
    digitalWrite(seg_con[2],LOW);
    digitalWrite(seg_con[1],LOW);
    digitalWrite(seg_con[0],HIGH);
    tampil_7(a);
    delay(5);
}

// Program cek mode (1. Jam Digital, 2. Stopwatch, 3. Timer countdown)
void cekMode()
{ 
  startJam = true;
  startStopwatch = false;
  startTimer = true;
  isProgramRun = false;
  ledOff();

  // Looping hingga dipilih mode dengan menekan tombol select
  while (!select)
  {
    show (10, 10, 10, mode);
    if (digitalRead(push[0])==0)
    {
      delay(150);
      mode++;
      if (mode>3)
      {
        mode=1;
      }
    }
    // Jika ditekan select maka akan keluar loop
    if (digitalRead(push[1])==0)
    {
      select = true;
    }
  }

  if (mode==2)
  {
    isStopwatch = true;
  }
}

void alarmON()
{
  digitalWrite(buzzer, HIGH);
  delay(120);
  digitalWrite(buzzer, LOW);
  delay(100);
  digitalWrite(buzzer, HIGH);
  delay(120);
  digitalWrite(buzzer, LOW);
  delay(100);
  digitalWrite(buzzer, HIGH);
  delay(120);
  digitalWrite(buzzer, LOW);
  delay(100);
  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer, LOW);
  delay(200);
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(100);
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(100);
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(600);
}

void alarmOFF()
{
  digitalWrite(buzzer, LOW);
}

// Program Running dari setiap mode yang dipilih
void runningProgram()
{
  signed char temp1;
  signed char temp2;
  signed char temp3;
  signed char temp4;
  int modeStopwatch = 0;
  
  switch (mode)
  {
    case 1 : {  // Jam Digital
                init_angka();
                setting_angka();
                isProgramRun = true;
                while (startJam)
                {
                  if (jam_puluhan==2 && jam_satuan>3)
                  {
                    init_angka();
                  }
                  show(jam_puluhan, jam_satuan, menit_puluhan, menit_satuan); 
                  if (digitalRead(push[1]) == 0)
                  {
                    setting_angka(); 
                    isProgramRun = true;
                  }
                  if (digitalRead(push[0]) == 0)
                  {
                    startJam = false; 
                  }
                }
             }
             break;
    case 2 : {  // Stopwatch
                while (isStopwatch)
                {
                    switch (modeStopwatch)
                    {
                      case 0 : {  // Tampilan sebelum mulai stopwatch.
                                  while(!startStopwatch)
                                  {
                                    init_angka();
                                    show(menit_puluhan, menit_satuan, detik_puluhan, detik_satuan); 
                                    if (digitalRead(push[2]) == 0) // Memulai stopwatch ketika ditekan tombol "plus"
                                    {
                                      startStopwatch = true; 
                                    }
                                    modeStopwatch = 1; 
                                  }
                               }
                               break;
                      case 1 : {
                                  halfSecond = 0;
                                  showFinalStopwatch = true;
                                  while (startStopwatch)
                                  {
                                    show(menit_puluhan, menit_satuan, detik_puluhan, detik_satuan); 
                                    if (digitalRead(push[0])==0)
                                    {
                                      isStopwatch = false;
                                      showFinalStopwatch = false;
                                      startStopwatch = false;
                                    }
                                    if (digitalRead(push[3]) == 0)
                                    {
                                      temp1 = menit_puluhan;
                                      temp2 = menit_satuan;
                                      temp3 = detik_puluhan;
                                      temp4 = detik_satuan;
                                      
                                      startStopwatch = false;
                                      modeStopwatch = 2; 
                                    }
                                  }
                               }
                               break;
                      case 2 : {
                                  while (showFinalStopwatch)
                                  {
                                     show (temp1, temp2, temp3, temp4);
                                     if (digitalRead(push[0])==0)
                                     {
                                        isStopwatch = false;
                                        showFinalStopwatch = false;
                                        startStopwatch = false;
                                     }
                                     if (digitalRead(push[2])==0)
                                     {
                                        showFinalStopwatch = false;
                                        startStopwatch = true;
                                        
                                        menit_puluhan = temp1;
                                        menit_satuan = temp2;
                                        detik_puluhan = temp3;
                                        detik_satuan = temp4;
                                        
                                        modeStopwatch = 1;
                                     }
                                  }
                               }
                               break;
                    } 
                }
             }
             break;
    case 3 : {  // Timer
                init_angka();
                setting_angka();
                isProgramRun = true;
                while (startTimer)
                {
                  show(timer_menitPuluhan, timer_menitSatuan, timer_detikPuluhan, timer_detikSatuan);
                  if (digitalRead(push[0])== 0)
                  {
                    startTimer = false;
                    doneTimer = false;
                  }
                  if (doneTimer)
                  {
                    startTimer = false;
                    doneTimer = true; 
                  }
                }
                while (doneTimer)
                {
                  //show(10,0,0,0);
                  for (loopAlarm=0; loopAlarm<3; loopAlarm++)
                  {
                    alarmON(); 
                  }
                  alarmOFF();
                  while (digitalRead(push[0])!=0)
                  {
                    show(10,10,10,10);
                  }
                  doneTimer = false;
                }
             }
             break;
  }

  select = false;
  
}

// Program Utama
void loop() 
{ 
  cekMode();
  runningProgram(); 
}
