#include <rxduino.h>

bool ChkDisconnectFlag = false;
int CntPDisconnected = 0;
static unsigned char DISCONNECT[] = "+DISCONNECTED=";
unsigned char Cdat;

//**************************************************
// ROBOBA005のセットアップ
// Serial1をオープン
//**************************************************
void ROBOBA005_Setup( void )
{
    Serial1.begin(115200, SCI_SCI2B);
    pinMode(PIN_P51 ,OUTPUT);

    digitalWrite( PIN_P51, 0 );
    delay(10);
    digitalWrite( PIN_P51, 1 );
    delay(10);
}

//**************************************************
// セットアップします
//**************************************************
void setup()
{
    pinMode( PIN_LED0, OUTPUT);
    pinMode( PIN_LED1, OUTPUT);
    pinMode( PIN_LED2, OUTPUT);
    pinMode( PIN_LED3, OUTPUT);

    Serial.begin(115200);
    //Serial.begin( 115200, SCI_AUTO );    	  //何か押されるまで待つ
    Serial.setDefault();                	//現在のシリアルポートを標準出力とするprintfでも出力する
    setvbuf(stdout,NULL,_IONBF,0);

    //ROBOBA005セットアップ
    ROBOBA005_Setup();

    //基板の青ボタンを有効にします
    pinMode(PIN_SW, INPUT);

    //接続待ちモードにします
    Serial1.print("AT+RESET\r\n");
    delay(1000);
    Serial1.print("AT+CLRPAIR\r\n");
    delay(1000);
    Serial1.print("AT+CONSLAVE2=1,1\r\n");
}

//**************************************************
// 無限ループしています
//**************************************************
void loop()
{
    while(Serial.available()) //何か受信があった
    {
        Cdat = (unsigned char)Serial.read(); //1文字取得
        Serial1.write( Cdat );
    }

    while(Serial1.available()) //何か受信があった
    {
        Cdat = (unsigned char)Serial1.read(); //1文字取得

        if( Cdat=='+' && ChkDisconnectFlag==false ){

            ChkDisconnectFlag = true;
            CntPDisconnected++;

        }
        else if( ChkDisconnectFlag==true && Cdat==DISCONNECT[CntPDisconnected] ){

            CntPDisconnected++;
            if( CntPDisconnected==14 ){

                //+DISCONNECTED確定！接続待ちモードにします
                Serial1.print("AT+RESET\r\n");
                delay(1000);
                Serial1.print("AT+CLRPAIR\r\n");
                delay(1000);
                Serial1.print("AT+CONSLAVE2=1,1\r\n");

                CntPDisconnected = 0;
                ChkDisconnectFlag = false;
            }
        }
        else{
            CntPDisconnected = 0;
            ChkDisconnectFlag = false;
        }        
        
        Serial.write( Cdat );
    }

    if( digitalRead(PIN_SW)==LOW ){
        
        //青ボタンが離されるまで待つ
        while(digitalRead(PIN_SW)==LOW);

        //通信を切断するために+++を送信
        Serial1.print( "+++" );
    }
}

