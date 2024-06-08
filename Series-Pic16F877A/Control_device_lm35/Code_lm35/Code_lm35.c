#include <16f877a.h> //Khai bao con PIC su dung
#device *=16 adc=10
//Cau hinh dao dong thach anh 20MHz
#fuses  HS,NOWDT,NOPROTECT,BROWNOUT,PUT,NOLVP
#use delay(clock=20M)
//khai bao thong so cong uart
#use rs232(baud=9600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8)
//----------------------Khai bao cac thu vien su dung-------------------------
#include<lcd.h>
#include <string.h>
//------------------------Dinh nghia cac chan ket noi-------------------------
//1. Khai bao khoi nut nhan
#define level_1 input(pin_B1) //so 1
#define level_2 input(pin_B2) //so 2
#define level_3 input(pin_B3) //so 3
#define mode    input(pin_B6) //che do
//2. Khai bao khoi relay
#define out_lv1  pin_C0 // relay_1
#define out_lv2  pin_C1 // relay_2
#define out_lv3  pin_C2 // relay_3
//3. Khai bao khoi tin hieu
#define led_mod     pin_B7//Led bao tin heu che do ON-AUTO, OFF-MANUAL
#define temp_sensor pin_A0//Cam bien nhiet do
//--------------------------Khai bao bien toan cuc----------------------------
int mod = 0; //Bien luu che do hoat dong 
unsigned int16 value; // gia tri bien tro
float value_adc; //gia tri dien ap

int1 check_level_1 = 0;
int1 check_level_2 = 0;
int1 check_level_3 = 0;

//------------------------Khai bao cac chuong trinh con-----------------------
//1. Chuong trinh ngat
#INT_EXT
void EXT_MODE(void)
{
   mod++;
   if(mod == 3)
      mod = 0;
}
//--------------------------------------
//2. Chuong trinh con
float Convert();
void _Auto(void);
void _Manual(void);
void _Manual1(void);
void _Level_1(void);
void _Level_2(void);
void _Level_3(void);
void dieuKhien(void);
void _Off(void);
void _General(void);
//----------------------------CHUONG TRINH CHINH------------------------------
void main()
{
   //Cai dat ban dau cho chuong trinh
   //1. Cai dat ngat
   enable_interrupts(INT_EXT);//Cho phep su dung chuong trinh ngat 
   ext_int_edge(L_TO_H);      // Ngat theo suon (+)  
   enable_interrupts(GLOBAL); //Cho phep ngat toan cuc
   //2. Chon che do cho cac port
   set_tris_a(0xff); //Thiet lap port A la input
   set_tris_b(0x7f); // thiet lap pin B0-B6 la input, B7 la output
   set_tris_c(0x0f); //Thiet lap port C la output
   set_tris_d(0x0f); //Thiet lap port D la output
   //3. Cai dat che do doc ADC
   setup_adc(ADC_CLOCK_INTERNAL);
   setup_adc_ports(ALL_ANALOG);
   set_adc_channel(0); //Doc 
   //4. Cai dat output ban dau
   output_bit(led_mod, 0); //Tat led mode (mode = 0 --> Manual)
   _Off();
   //5. Khoi tao LCD
   Convert();
   lcd_init();
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   lcd_putc(" HE THONG THONG ");
   lcd_gotoxy(1,2);
   lcd_putc(" GIO CHO GARAGES");
   delay_ms(7000);
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   printf(lcd_putc,"NGUYEN VAN THAI");
   lcd_gotoxy(1,2);
   printf(lcd_putc,"NHIETDO: %3.1f C", Convert());
   delay_ms(200);
   while(TRUE)
   {
      lcd_gotoxy(1,1);
      printf(lcd_putc,"NGUYEN VAN THAI");
      lcd_gotoxy(1,2);
      printf(lcd_putc,"NHIETDO: %3.1f C", Convert());
      delay_ms(200);
      //TODO: User Code
      switch (mod)
      {
         case 0: _Manual1(); break; //Neu mod = 0 --> Manual
         case 1: _Auto(); break;   //Neu mod = 1 --> Auto
         default: _Manual1(); break;//Che do mac dinh: Manual
      }     
   }
}
//-------------------Trien khai cac chuong trinh con--------------------------
float Convert()
{
   value = read_adc();
   /*
   lm35 quy dinh 10mv---------->1 do C
   */
   return (float)value*0.488-40;
//!   return value*5.0f/1023.0f;
}
void _Manual(void)//Che do dieu khien tay
{
   output_bit(led_mod, 0); 
   while(level_1 == 0)
   {
      output_bit(out_lv1, 1);
   }
   while(level_2 == 0)
   {
      output_bit(out_lv2, 1);
   }
   while(level_3 == 0)
   {
      output_bit(out_lv3, 1);
   }
}

void _Manual1(void)//Che do dieu khien tay
{
   output_bit(led_mod, 0); 
   
   if(level_1 == 0){
      delay_ms(20);
      if(level_1 == 0){
         check_level_1 = ~check_level_1;
      }
      while(level_1 == 0);
   }
   if(level_2 == 0){
      delay_ms(20);
      if(level_2 == 0){
         check_level_2 = ~check_level_2;
      }
      while(level_2 == 0);
   }
   if(level_3 == 0){
      delay_ms(20);
      if(level_3 == 0){
         check_level_3 = ~check_level_3;
      }
      while(level_3 == 0);
   }
   
   dieuKhien();
   
}

void dieuKhien(){

   if(check_level_1 == 1){
      output_bit(out_lv1, 1);
   }else {
      output_bit(out_lv1, 0);
   }

   if(check_level_2 == 1){
      output_bit(out_lv2, 1);
   }else {
      output_bit(out_lv2, 0);
   }

   if(check_level_3 == 1){
      output_bit(out_lv3, 1);
   }else {
      output_bit(out_lv3, 0);
   }
}

//Che do tu dong
void _Auto(void)
{
   output_bit(led_mod, 1);
   value_adc = Convert();
   if(value_adc < 20) _Level_1();//30
   else if (value_adc < 30) _Level_2();
   else if (value_adc <= 100) _Level_3();
}
//So 1
void _Level_1(void)
{
   output_bit(out_lv1, 1);
   output_bit(out_lv2, 0);
   output_bit(out_lv3, 0);
}
//So 2
void _Level_2(void)
{
   output_bit(out_lv1, 0);
   output_bit(out_lv2, 1);
   output_bit(out_lv3, 1);
}
//So 3
void _Level_3(void)
{
   output_bit(out_lv1, 1);
   output_bit(out_lv2, 1); 
   output_bit(out_lv3, 1);
}
//Tat het
void _Off(void)
{
   output_bit(out_lv1, 0);
   output_bit(out_lv2, 0);
   output_bit(out_lv3, 0);
}
//----------------------------------------------------------------------------
