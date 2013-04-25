#include <rxduino.h>

#define MAX_UART_CHAR 64
int UartCharMojisu;                        //シリアルから取得した文字数(MAX_UART_CHARを超えないこと)
unsigned char UartChar[MAX_UART_CHAR];
int UartReadNowFlg = 0;                    //シリアルからの受信中フラグ

#define DISCONNECTED_CNT 14
#define CONNECTED_CNT 11
static unsigned char DISCONNECTED[] = "+DISCONNECTED=";
static unsigned char CONNECTED[] = "+CONNECTED=";

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
    //ROBOBA005用にSerial1をオープン
    Serial1.begin(115200, SCI_SCI2B);

    pinMode(PIN_P51 ,OUTPUT);

    //ROBOBA005をリセット
    digitalWrite( PIN_P51, 0 );
    delay(10);

    //ROBOBA005をリセット解除
    digitalWrite( PIN_P51, 1 );
    delay(10);
}

//***************************************************************************
// uartCallFuncの初期化
//***************************************************************************
void uartCallFuncIni( void ) 
{
  UartChar[0] = '\0';   //あまり意味無い
  UartCharMojisu = 0;   //uart1からの読み込み文字数の初期化
  UartReadNowFlg = 0;   //受信中フラグの初期化
}

//*****************************************************************************
// 受信した内容を解析して、処理関数を呼び出します
//
// 開始コード: 'A'  終了コード: 0x0A
//*****************************************************************************
void readCommand()
{
    while(Serial.available()) //何か受信があった
    {
        unsigned char c = (unsigned char)Serial.read(); //1文字取得

        //デバッグ用エコー
        //Serial.write( c );

        if( UartCharMojisu==0 && c=='A'){    //'A'がスタートコードという設定

            //配列に読み込む
            UartChar[UartCharMojisu] = c;
            UartCharMojisu++;

            UartReadNowFlg = 1; //受信開始

        }else if( ( UartReadNowFlg==1 && c!=0x0A && UartCharMojisu == sizeof(UartChar)-2)){            //c==0x0A:終了コード, 
        
            Serial.println( "Command Too Long." );

            UartCharMojisu = 0;
            UartReadNowFlg = 0; //受信中止

        }else if( UartReadNowFlg==1 && c==0x0A && UartCharMojisu>0 ){

            UartReadNowFlg = 0; //受信中止

            //コマンドを受け付けた(改行コード0x0D 0x0Aを追加する)
            UartChar[UartCharMojisu] = 0x0D;
            UartCharMojisu++;
            UartChar[UartCharMojisu] = 0x0A;
            UartCharMojisu++;

            //コマンドの送信
            Serial1.write( UartChar, UartCharMojisu );
            
            UartCharMojisu = 0;

        }else if( UartReadNowFlg==1 ){
            //配列に読み込む
            UartChar[UartCharMojisu] = c;
            UartCharMojisu++;
        }
    }
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
    //Serial.begin( 115200, SCI_AUTO );    //何か押されるまで待つ
    Serial.setDefault();                //現在のシリアルポートを標準出力とするprintfでも出力する
    setvbuf(stdout,NULL,_IONBF,0);

    //ROBOBA005セットアップ
    ROBOBA005_Setup();

    //基板の青ボタンを有効にします
    pinMode(PIN_SW, INPUT);

    //uartCallFuncの初期化
    uartCallFuncIni();

    //接続待ちモードにします
    //Serial1.print("AT+RESET\r\n");
    //delay(1000);
    //Serial1.print("AT+CLRPAIR\r\n");
    //delay(1000);
    //Serial1.print("AT+CONSLAVE2=1,1\r\n");
    //CommandMode = false;
    
    CommandMode = true;
}

//**************************************************
// 無限ループしています
//**************************************************
void loop()
{
    if(CommandMode==true){
        readCommand();
    }
    else{
        while(Serial.available()) //何か受信があった
        {
            Ct = (unsigned char)Serial.read(); //1文字取得
            Serial1.write( Ct );

            //Serial.write( Ct );
        }
    }

    while(Serial1.available()) //何か受信があった
    {
        Ct = (unsigned char)Serial1.read(); //1文字取得

        if( Ct=='+' && ChkConnectFlag==false ){

            ChkConnectFlag = true;
            CntConnect++;

        }
        else if( ChkConnectFlag==true && Ct==CONNECTED[CntConnect] ){

            CntConnect++;
            if( CntConnect==CONNECTED_CNT ){

                //+CONNECTED確定！通信モードにする
                CntConnect = 0;
                ChkConnectFlag = false;
                CommandMode = false;
            }
        }
        else if( ChkConnectFlag==true && Ct==DISCONNECTED[CntConnect] ){

            CntConnect++;
            if( CntConnect==DISCONNECTED_CNT ){

                //+DISCONNECTED確定！コマンドモードにする
                CntConnect = 0;
                ChkConnectFlag = false;
                CommandMode = true;
            }
        }
        else{
            CntConnect = 0;
            ChkConnectFlag = false;
        }
        Serial.write( Ct );
    }

    //基板の青ボタンを押して測定を開始させます
    if( digitalRead(PIN_SW)==LOW ){
        
        //青ボタンが離されるまで待つ
        while(digitalRead(PIN_SW)==LOW);

        Serial1.print( "+++" );

        //コマンドモードに戻す
        CommandMode = true;
        UartCharMojisu = 0;
        UartReadNowFlg = 0; //受信中止
    }
}

