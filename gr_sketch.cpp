#include <rxduino.h>

#define DISCONNECTED_CNT 14
static unsigned char DISCONNECTED[] = "+DISCONNECTED=";

bool CommandMode = true;
bool ChkConnectFlag = false;
int CntConnect = 0;
unsigned char Ct;

//**************************************************
// ROBOBA005のセットアップ
// Serial1をオープン
//**************************************************
void ROBOBA005_Setup( void )
{
	//XBee用にSerial1をオープン
	Serial1.begin(115200, SCI_SCI2B);

	pinMode(PIN_P51 ,OUTPUT);

	//XBeeをリセット
	digitalWrite( PIN_P51, 0 );
    delay(10);

	//XBeeをリセット解除
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
    //Serial.begin( 115200, SCI_AUTO );	//何か押されるまで待つ
	Serial.setDefault();				//現在のシリアルポートを標準出力とするprintfでも出力する
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
		Ct = (unsigned char)Serial.read(); //1文字取得
		Serial1.write( Ct );
	}

	while(Serial1.available()) //何か受信があった
	{
		Ct = (unsigned char)Serial1.read(); //1文字取得

		if( Ct=='+' && ChkConnectFlag==false ){

			ChkConnectFlag = true;
			CntConnect++;

		}
		else if( ChkConnectFlag==true && Ct==DISCONNECTED[CntConnect] ){
			CntConnect++;
			if( CntConnect==DISCONNECTED_CNT ){

				//+DISCONNECTED確定！
				//接続待ちモードにします
				Serial1.print("AT+RESET\r\n");
				delay(1000);
				Serial1.print("AT+CLRPAIR\r\n");
				delay(1000);
				Serial1.print("AT+CONSLAVE2=1,1\r\n");

				CntConnect = 0;
				ChkConnectFlag = false;
			}
		}
		else{
			CntConnect = 0;
			ChkConnectFlag = false;
		}
		Serial.write( Ct );
	}

	//基板の青ボタンを押して接続を切ります
	if( digitalRead(PIN_SW)==LOW ){
		
		//青ボタンが離されるまで待つ
		while(digitalRead(PIN_SW)==LOW);

		Serial1.print( "+++" );
	}
}
