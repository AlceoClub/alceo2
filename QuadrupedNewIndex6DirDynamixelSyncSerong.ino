#include <DynamixelSerial.h>  //10 500

#define Zoff 37   //45
#define coxa 30
#define femur 43
#define tibia 50
#define lebarLangkah 40     // lebar langkah dibagi rate*2 harus bulat40 4 32
#define rate 4             // ketelitian langkah
#define tinggiLangkah 20    // tinggi langkah dibagi rate harus bulat

#define endCom()        (Serial.end())          // End Serial Comunication
#define switchCom(DirPin,Mode) (digitalWrite(DirPin,Mode))  // Switch to TX/RX Mode
#define pin 2
#define Tx_MODE 1
#define Rx_MODE 0
#define numServo 12

#define pin1 9
#define pin2 10
#define pin3 11
#define pin4 12

#define delayJalanTempat 25

int input=0,inputBefore=0;
int delayKecepatan=10;
int speeds=500;
void syncLeg();

float posisiAwal=((coxa+femur)/1.3);    //1.5
float langkahDatar[20];
float langkahNaik=(tinggiLangkah/rate);
int jumlahPosisiKaki=((rate*2)+1);
int rate1=(rate+1);
int rate2=((rate*2)+1);
int rate3=((rate*3)+1);
int rate4=((rate*4)+1);
int ubahGerak=0;
int prepareZ=0;
int indexKanan=0, indexKiri=0;

int angka=0;

int angle[12];

int panjangData;
unsigned char all[100],checksum;

struct leg
{
    int sudutDalam;
    int sudutTengah;
    int sudutLuar;
    int posisi;
    int motion;   // ada 4 motion
    int gerakan;  // 0 = gerak segitiga     1 = dorong
    float posisiX;
    float posisiY;
    float posisiZ;
}leg[4];

void inisialisasiAwal()
{
  for(int a=0;a<4;a++)
  {
      leg[a].posisiX=posisiAwal;
      leg[a].posisiY=posisiAwal;
      leg[a].posisiZ=0;
      leg[a].posisi=jumlahPosisiKaki;
      leg[a].gerakan=1;
      inverse(a,leg[a].posisiX,leg[a].posisiY,leg[a].posisiZ);
  }
}

void setup() {
  // put your setup code here, to run once:
    pinMode(pin,OUTPUT);
    pinMode(pin1,INPUT_PULLUP);
    pinMode(pin2,INPUT_PULLUP);
    pinMode(pin3,INPUT_PULLUP);
    pinMode(pin4,INPUT_PULLUP);
    Serial.begin(1000000);
    langkahDatar[0]=(lebarLangkah/(rate*2));
    langkahDatar[1]=(lebarLangkah/(rate*(1.9)));      //ke arah 1 kanan
    langkahDatar[2]=(lebarLangkah/(rate*(2.1)));   //ke arah 1 kiri
    langkahDatar[3]=(lebarLangkah/(rate*(2)));    //ke arah 4 kanan
    langkahDatar[4]=(lebarLangkah/(rate*(1.8)));    //ke arah 4 kiri
    langkahDatar[5]=(lebarLangkah/(rate*(1.7)));    //ke arah 2 kanan
    langkahDatar[6]=(lebarLangkah/(rate*(2.1)));    //ke arah 2 kiri
    langkahDatar[7]=(lebarLangkah/(rate*(1.8)));    //ke arah 3 kanan
    langkahDatar[8]=(lebarLangkah/(rate*(1.9)));      //ke arah 3 kiri

    langkahDatar[9]=(lebarLangkah/(rate*(1.8)));      //ke arah 1 kanan
    langkahDatar[10]=(lebarLangkah/(rate*(2.8)));   //ke arah 1 kiri
    langkahDatar[11]=(lebarLangkah/(rate*(2.6)));    //ke arah 4 kanan
    langkahDatar[12]=(lebarLangkah/(rate*(2.4)));    //ke arah 4 kiri
    langkahDatar[13]=(lebarLangkah/(rate*(2.3)));    //ke arah 2 kanan
    langkahDatar[14]=(lebarLangkah/(rate*(2.7)));    //ke arah 2 kiri
    langkahDatar[15]=(lebarLangkah/(rate*(2.5)));    //ke arah 3 kanan
    langkahDatar[16]=(lebarLangkah/(rate*(2.4)));      //ke arah 3 kiri

    langkahDatar[17]=(lebarLangkah/(rate*(3))); //pivot Api
    panjangData=(5*numServo)+4;
    all[0]=0xFF;
    all[1]=0xFF;
    all[2]=0xFE;
    all[3]=panjangData;
    all[4]=0x83;
    all[5]=0x1E;
    all[6]=0x04;
    inisialisasiAwal();
    syncLeg();
}

long rad2deg(float rad)
{
    return ((int)(rad*57.29577957855));
}

int convert(int deg)
{
    int zz=map(deg, 0, 300, 0, 1024);//((int)(deg*(1024/300)));
    return zz;
}

void syncWrite()
{
  for(int a=0;a<numServo;a++)
  {
    all[7+(5*a)]=a;
    all[8+(5*a)]=angle[a]&0xFF;
    all[9+(5*a)]=angle[a]>>8;
    all[10+(5*a)]=speeds&0xFF;
    all[11+(5*a)]=speeds>>8;
  }
  checksum=0;
  for(int a=2;a<=panjangData+2;a++)
  {
    checksum += all[a];
  }
  all[panjangData+3]=~checksum;

  switchCom(pin,Tx_MODE);
  Serial.write(all,panjangData+4);
  delayMicroseconds(100);
  switchCom(pin,Rx_MODE);
}

void setServo(int idLeg, int sudut1, int sudut2, int sudut3)
{
  if(idLeg==0)
  {
    angle[0]=convert((105+sudut1));
    angle[1]=convert((240-sudut2));
    angle[2]=convert((60+sudut3));
  }
  else if(idLeg==1)
  {
    angle[3]=convert((105+sudut1));
    angle[4]=convert((240-sudut2));
    angle[5]=convert((60+sudut3));
  }
  else if(idLeg==2)
  {
    angle[6]=convert((105+sudut1));
    angle[7]=convert((60+sudut2));
    angle[8]=convert((240-sudut3));
  }
  else if(idLeg==3)
  {
    angle[9]=convert((105+sudut1));
    angle[10]=convert((60+sudut2));
    angle[11]=convert((240-sudut3));
  }
//  Serial.println("Sudut:");
//  Serial.println(idLeg);
//  Serial.println(sudut1);
//  Serial.println(sudut2);
//  Serial.println(sudut3);
}

void syncLeg()
{
  setServo(0,leg[0].sudutDalam,leg[0].sudutTengah,leg[0].sudutLuar);
  setServo(1,leg[1].sudutDalam,leg[1].sudutTengah,leg[1].sudutLuar);
  setServo(2,leg[2].sudutDalam,leg[2].sudutTengah,leg[2].sudutLuar);
  setServo(3,leg[3].sudutDalam,leg[3].sudutTengah,leg[3].sudutLuar);
  syncWrite();
}

void inverse(int idLeg, float x, float y, float z)
{
  float degree1,degree2,degree3;
  float degree2_1,degree2_2;
  float L1,L1_2,L,L_2;
  float Z,Lcox;
  float temp2,temp2_2,temp2_3,temp2_4;
  float temp3,temp3_2,temp3_3,temp3_4;
  float femur_2,tibia_2;
  int sudut1,sudut2,sudut3;
  
  degree1=atan(x/y);
  L1_2=((x*x)+(y*y));
  L1=sqrt(L1_2);
  Z=Zoff-z;
  Lcox=L1-coxa;
  L_2=((Z*Z)+(Lcox*Lcox));
  L=sqrt(L_2);
  degree2_1=atan(Lcox/Z);

  femur_2=femur*femur;
  tibia_2=tibia*tibia;
  temp2=((L_2+femur_2-tibia_2)/(2*femur*L));
  temp2_2=temp2*temp2;
  temp2_3=1-temp2_2;
  if(temp2_3<0)
  {
    temp2_3=(temp2_3*(-1));
  }
  temp2_4=sqrt(temp2_3);
  degree2_2=atan(temp2_4/temp2);  
  degree2=degree2_1+degree2_2;

  temp3=((2*L*femur)/(femur_2+tibia_2-L_2));
  temp3_2=sin(degree2_2);
  degree3=atan((temp3*temp3_2));

  sudut1=rad2deg(degree1);
  sudut2=rad2deg(degree2);
  sudut3=rad2deg(degree3);
  if(sudut3<0)
  {
    sudut3=180+sudut3;
  }
  leg[idLeg].sudutDalam=sudut1;
  leg[idLeg].sudutTengah=sudut2;
  leg[idLeg].sudutLuar=sudut3;
}

void motion(int idLeg, int indexLebar)
{
  //Serial.println(idLeg);
  if(leg[idLeg].motion==0)  // ke Y positif
  {
    if(leg[idLeg].gerakan==0)    //gerak segitiga
    {
      if(leg[idLeg].posisi<rate1)//(rate+1))
      {
        leg[idLeg].posisiY+=langkahDatar[indexLebar];
        leg[idLeg].posisiZ+=langkahNaik;
        leg[idLeg].posisi++;
      }
      else if(leg[idLeg].posisi>=rate1 && leg[idLeg].posisi<rate2)  //leg[idLeg].posisi>=(rate+1) && leg[idLeg].posisi<((rate*2)+1)
      {
        leg[idLeg].posisiY+=langkahDatar[indexLebar];
        leg[idLeg].posisiZ-=langkahNaik;
        leg[idLeg].posisi++;
      }
      if(leg[idLeg].posisi==rate2)
      {
        leg[idLeg].gerakan=1;
      }
        
    }
    else if(leg[idLeg].gerakan==1)  //dorong
    {
      if(leg[idLeg].posisi>1)
      {
        leg[idLeg].posisiY-=langkahDatar[indexLebar];
        leg[idLeg].posisi--;
      }
      if(leg[idLeg].posisi==1)
      {
        leg[idLeg].gerakan=0;
      }
    }
  }
  
  else if(leg[idLeg].motion==1)  // ke Y negatif
  {
    if(leg[idLeg].gerakan==0)    //gerak segitiga
    {
      if(leg[idLeg].posisi>rate1)//(rate+1))
      {
        leg[idLeg].posisiY-=langkahDatar[indexLebar];
        leg[idLeg].posisiZ+=langkahNaik;
        leg[idLeg].posisi--;
      }      
      else if(leg[idLeg].posisi<=rate1)//(rate+1))
      {
        leg[idLeg].posisiY-=langkahDatar[indexLebar];
        leg[idLeg].posisiZ-=langkahNaik;
        leg[idLeg].posisi--;
      }
      if(leg[idLeg].posisi==1)
      {
        leg[idLeg].gerakan=1;
      }
        
    }
    else if(leg[idLeg].gerakan==1)  //dorong
    {
      if(leg[idLeg].posisi<rate2)//((rate*2)+1))
      {
        leg[idLeg].posisiY+=langkahDatar[indexLebar];
        leg[idLeg].posisi++;
      }
      if(leg[idLeg].posisi==rate2)//((rate*2)+1))
      {
        leg[idLeg].gerakan=0;
      }
    }
  }
  
  else if(leg[idLeg].motion==2)  // ke X negatif
  {
    if(leg[idLeg].gerakan==0)    //gerak segitiga
    {
      if(leg[idLeg].posisi<rate3)//((rate*3)+1))
      {
        leg[idLeg].posisiX-=langkahDatar[indexLebar];
        leg[idLeg].posisiZ+=langkahNaik;
        leg[idLeg].posisi++;
      }  
      else if(leg[idLeg].posisi>=rate3)//((rate*3)+1))
      {
        leg[idLeg].posisiX-=langkahDatar[indexLebar];
        leg[idLeg].posisiZ-=langkahNaik;
        leg[idLeg].posisi++;
      }
      if(leg[idLeg].posisi==rate4)//((rate*4)+1))
      {
        leg[idLeg].gerakan=1;
      }
    }
    else if(leg[idLeg].gerakan==1)  //dorong
    {
      if(leg[idLeg].posisi>rate2)//((rate*2)+1))
      {
        leg[idLeg].posisiX+=langkahDatar[indexLebar];
        leg[idLeg].posisi--;
      }
      if(leg[idLeg].posisi==rate2)//((rate*2)+1))
      {
        leg[idLeg].gerakan=0;
      }
    }
  }
  
  else if(leg[idLeg].motion==3)  // ke X positif
  {
    if(leg[idLeg].gerakan==0)    //gerak segitiga
    {
      if(leg[idLeg].posisi>rate3)//((rate*3)+1))
      {
        leg[idLeg].posisiX+=langkahDatar[indexLebar];
        leg[idLeg].posisiZ+=langkahNaik;
        leg[idLeg].posisi--;
      }
      else if(leg[idLeg].posisi<=rate3)//((rate*3)+1))
      {
        leg[idLeg].posisiX+=langkahDatar[indexLebar];
        leg[idLeg].posisiZ-=langkahNaik;
        leg[idLeg].posisi--;
      }
      if(leg[idLeg].posisi==rate2)//((rate*2)+1))
      {
        leg[idLeg].gerakan=1;
      }
        
    }
    else if(leg[idLeg].gerakan==1)  //dorong
    {
      if(leg[idLeg].posisi<rate4)//((rate*4)+1))
      {
        leg[idLeg].posisiX-=langkahDatar[indexLebar];
        leg[idLeg].posisi++;
      }
      if(leg[idLeg].posisi==rate4)//((rate*4)+1))
      {
        leg[idLeg].gerakan=0;
      }
    }
  }
  
  inverse(idLeg,leg[idLeg].posisiX,leg[idLeg].posisiY,leg[idLeg].posisiZ);
  //delay(50);
}

void tuning()
{
  setServo(0,45,90,90);
  setServo(1,45,90,90);
  setServo(2,45,90,90);
  setServo(3,45,90,90);
  syncWrite();
}

void directions(int kananD, int kananB, int kiriB, int kiriD)
{
  if(inputBefore==1)
  {
    indexKanan=1;indexKiri=2;
  }
  else if(inputBefore==2)
  {
    indexKanan=5;indexKiri=6;
  }
  else if(inputBefore==3)
  {
    indexKanan=7;indexKiri=8;
  }
  else if(inputBefore==4)
  {
    indexKanan=3;indexKiri=4;
  }
//  else if(inputBefore==8)
//  {
//    indexKanan=9;indexKiri=2;
//  }
//  else if(inputBefore==9)
//  {
//    indexKanan=1;indexKiri=10;
//  }
//  else if(inputBefore==10)
//  {
//    indexKanan=13;indexKiri=6;
//  }
//  else if(inputBefore==11)
//  {
//    indexKanan=5;indexKiri=14;
//  }
//  else if(inputBefore==12)
//  {
//    indexKanan=15;indexKiri=8;
//  }
//  else if(inputBefore==13)
//  {
//    indexKanan=7;indexKiri=16;
//  }
//  else if(inputBefore==14)
//  {
//    indexKanan=11;indexKiri=4;
//  }
//  else if(inputBefore==15)
//  {
//    indexKanan=3;indexKiri=12;
//  }
  if(ubahGerak==0)
  {
      while(leg[kananD].posisi!=1)
      {
        leg[kananD].motion=1;
        motion(kananD,indexKanan);
        syncLeg();
      }
      while(leg[kananB].posisi!=rate4)//((rate*4)+1))
      {
        leg[kananB].motion=2;
        motion(kananB,indexKanan);
        syncLeg();
      }
      ubahGerak=1;
  }
  leg[kananD].motion=0;
  leg[kananB].motion=2;
  leg[kiriB].motion=1;
  leg[kiriD].motion=3;
  motion(kananD,indexKanan);motion(kananB,indexKanan);motion(kiriB,indexKiri);motion(kiriD,indexKiri);
  syncLeg();
}

void pivot(int arah)
{
  if(arah==0) //putar kiri
  {
    if(ubahGerak==0)
    {
      while(leg[0].posisi!=rate4&&leg[2].posisi!=rate4)
      {
        leg[0].motion=2;
        motion(0,0);
        leg[2].motion=2;
        motion(2,0);
        syncLeg();
      }
      ubahGerak=1;
    }
    leg[0].motion=2;
    leg[1].motion=2;
    leg[2].motion=2;
    leg[3].motion=2;
    motion(0,0);motion(1,0);motion(2,0);motion(3,0);
    syncLeg();
  }
  else if(arah==1) //putar kanan
  {
    if(ubahGerak==0)
    {
      while(leg[1].posisi!=1&&leg[3].posisi!=1)
      {
        leg[1].motion=1;
        motion(1,0);
        leg[3].motion=1;
        motion(3,0);
        syncLeg();
      }
      ubahGerak=1;
    }
    leg[0].motion=1;
    leg[1].motion=1;
    leg[2].motion=1;
    leg[3].motion=1;
    motion(0,0);motion(1,0);motion(2,0);motion(3,0);
    syncLeg();
  }
}

void pivotApi(int arah)
{
  if(arah==0) //putar kiri
  {
    if(ubahGerak==0)
    {
      while(leg[0].posisi!=rate4&&leg[2].posisi!=rate4)
      {
        leg[0].motion=2;
        motion(0,17);
        leg[2].motion=2;
        motion(2,17);
        syncLeg();
      }
      ubahGerak=1;
    }
    leg[0].motion=2;
    leg[1].motion=2;
    leg[2].motion=2;
    leg[3].motion=2;
    motion(0,17);motion(1,17);motion(2,17);motion(3,17);
    syncLeg();
  }
  else if(arah==1) //putar kanan
  {
    if(ubahGerak==0)
    {
      while(leg[1].posisi!=1&&leg[3].posisi!=1)
      {
        leg[1].motion=1;
        motion(1,17);
        leg[3].motion=1;
        motion(3,17);
        syncLeg();
      }
      ubahGerak=1;
    }
    leg[0].motion=1;
    leg[1].motion=1;
    leg[2].motion=1;
    leg[3].motion=1;
    motion(0,17);motion(1,17);motion(2,17);motion(3,17);
    syncLeg();
  }
}

void jalanTempat()
{
  for(int a=0;a<rate;a++)
  {
      leg[0].posisiZ+=langkahNaik;
      leg[2].posisiZ+=langkahNaik;
      inverse(0,leg[0].posisiX,leg[0].posisiY,leg[0].posisiZ);
      inverse(2,leg[2].posisiX,leg[2].posisiY,leg[2].posisiZ);
      syncLeg();
      delay(delayJalanTempat);
  }
  for(int a=0;a<rate;a++)
  {
      leg[0].posisiZ-=langkahNaik;
      leg[2].posisiZ-=langkahNaik;
      inverse(0,leg[0].posisiX,leg[0].posisiY,leg[0].posisiZ);
      inverse(2,leg[2].posisiX,leg[2].posisiY,leg[2].posisiZ);
      syncLeg();
      delay(delayJalanTempat);
  }
  for(int a=0;a<rate;a++)
  {
      leg[1].posisiZ+=langkahNaik;
      leg[3].posisiZ+=langkahNaik;
      inverse(1,leg[1].posisiX,leg[1].posisiY,leg[1].posisiZ);
      inverse(3,leg[3].posisiX,leg[3].posisiY,leg[3].posisiZ);
      syncLeg();
      delay(delayJalanTempat);
  }
  for(int a=0;a<rate;a++)
  {
      leg[1].posisiZ-=langkahNaik;
      leg[3].posisiZ-=langkahNaik;
      inverse(1,leg[1].posisiX,leg[1].posisiY,leg[1].posisiZ);
      inverse(3,leg[3].posisiX,leg[3].posisiY,leg[3].posisiZ);
      syncLeg();
      delay(delayJalanTempat);
  }
}

void cekPerintah()
{
  input=0;
  if(leg[0].posisi==rate2 || leg[1].posisi==rate2 || leg[2].posisi==rate2 || leg[3].posisi==rate2)
  {
      if(digitalRead(pin1)==LOW||digitalRead(pin2)==LOW||digitalRead(pin3)==LOW||digitalRead(pin4)==LOW)
      {
          if(digitalRead(pin1)==LOW)
            input+=1;
          if(digitalRead(pin2)==LOW)
            input+=2;
          if(digitalRead(pin3)==LOW)
            input+=4;
          if(digitalRead(pin4)==LOW)
            input+=8;
      }
      if((input!=inputBefore))//&&input<10&&inputBefore<10)
      {
        ubahGerak=0;
        inisialisasiAwal();
        inputBefore=input;
      }
//      else if(input>9)
//      {
//        inputBefore=input;
//      }
//      else if(inputBefore>9&&input<10)
//      {
//        inputBefore=input;
//      }
  }
  if(inputBefore==0)syncLeg();
  else if(inputBefore==1)directions(0,1,2,3);
  else if(inputBefore==2)directions(1,2,3,0);
  else if(inputBefore==3)directions(2,3,0,1);
  else if(inputBefore==4)directions(3,0,1,2);
  else if(inputBefore==5)pivotApi(0);
  else if(inputBefore==6)pivotApi(1);
  else if(inputBefore==7)jalanTempat();
  else if(inputBefore==8)pivotApi(1);
  else if(inputBefore==9)pivotApi(0);
  else if(inputBefore==10){delayKecepatan=20;}
  else if(inputBefore==11){delayKecepatan=10;}
  else if(inputBefore==12){delayKecepatan=15;}
//  
//  else if(inputBefore==8)directions(0,1,2,3);
//  else if(inputBefore==9)directions(0,1,2,3);
//  else if(inputBefore==10)directions(1,2,3,0);
//  else if(inputBefore==11)directions(1,2,3,0);
//  else if(inputBefore==12)directions(2,3,0,1);
//  else if(inputBefore==13)directions(2,3,0,1);
//  else if(inputBefore==14)directions(3,0,1,2);
//  else if(inputBefore==15)directions(3,0,1,2);
//  else if(inputBefore==16)pivotApi(1);
//  else if(inputBefore==17)pivotApi(0);
//  else if(inputBefore==18){delayKecepatan=20;}
//  else if(inputBefore==19){delayKecepatan=10;}
//  else if(inputBefore==20){delayKecepatan=5;}
  else syncLeg();
  delay(delayKecepatan); 
}

void loop() {
//  syncLeg();
  cekPerintah();
//  jalanTempat();
//inputBefore=1;
//directions(0,1,2,3);
//delay(10); 
}
