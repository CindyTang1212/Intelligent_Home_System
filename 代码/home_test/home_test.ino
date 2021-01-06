#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2     //DS18B20接至Arduino数字口2
#define MQ2_Sensor_A0 A0   //MQ-2传感器模拟输出接Arduino模拟A0口
#define MQ2_Sensor_D0 3    //MQ-2传感器数字输出接Arduino数字口3
#define SW 4               //继电器模块接至Arduino数字口4
#define infrared_Pin 7     //人体红外感应模块接Arduino数字口7
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define gas_Command        0x82   
#define temp_Command       0x81   
#define infrared_Command   0x80   
#define SW_ON_Command      0x11   
#define SW_OFF_Command     0x10  

int pirPin = 7;

#include <SPI.h>
#include <Ethernet.h>
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,101,177);
EthernetServer server(8000);

byte comdata[3]={0};
void data_handle(void);    //测试串口数据是否正确并执行命令
int sec = 0;

void setup()
{
  Ethernet.begin(mac, ip);
  server.begin();
  sensors.begin();
  pinMode(infrared_Pin, INPUT); 
  pinMode(SW, OUTPUT); 
  Serial.begin(9600);
}
  
void loop()
{
   int i ;
  EthernetClient client = server.available();
  if (client) {
    Serial.print("yes!\n");
    while (client.connected()) {
      if (client.available()) { 
         for(i=0;i<3;i++)
         {
              comdata[i] =client.read();
               //延时一会，让串口缓存准备好下一个字节，不延时可能会导致数据丢失
              delay(10);
           }
          data_handle();
        }
      }
    client.stop();
  }
}
void data_handle()
{
  if(comdata[0] == 0x55)    //0x55和0xAA均为判断是否为有效命令
   {
     if(comdata[1] == 0xAA)
     {
       switch (comdata[2])
        {
            case SW_ON_Command: 
            Serial.print("You turned on the switch!\n"); 
            digitalWrite(SW, HIGH);        
            break;
            
            case SW_OFF_Command:  
            Serial.print("You turned off the switch!\n");
            digitalWrite(SW, LOW);        
            break;
            
            case gas_Command:
            Serial.print("mq-2 module: ");
            Serial.print(analogRead(A0)); 
            Serial.print("\n"); 
            server.print(analogRead(A0));
            break;
            
            case temp_Command: 
            Serial.print("Now we're testing ds18b20 module!\n");
            sensors.requestTemperatures();
            server.print(sensors.getTempCByIndex(0),2);    
            break;
            
            case infrared_Command:
            int second = 0;
            int alarm = false;
            while(second < 10){
              if(digitalRead(pirPin) != 0){
                alarm = true;
                break;
              }
              delay(1000);  
              second++;        
            }
            if(alarm){
              Serial.print("Oh no!");
              server.print(1);
            }
            else{
              Serial.print("Safe!");
              server.print(0);
            }
            Serial.print("\n");
            break;
          
        }
      }
   }
}     
