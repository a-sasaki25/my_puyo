//課題
//2020/06/04

#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <unistd.h>

//ぷよの色を表すの列挙型
//NONEが無し，RED,BLUE,..が色を表す
enum puyocolor { NONE, RED, BLUE, GREEN, YELLOW};

class PuyoArray
{
private:
  //盤面状態
  puyocolor *data;
  unsigned int data_line;
  unsigned int data_column;

  //メモリ開放
  void Release()
  {
    if (data == NULL) {
      return;
    }

    delete[] data;
    data = NULL;
  }

public:
  //コンストラクタ
  PuyoArray(){data=NULL, data_line=0, data_column=0;}
  //デストラクタ
  ~PuyoArray(){Release();}
    
  //盤面サイズ変更
  void ChangeSize(unsigned int line, unsigned int column)
  {
    Release();

    //新しいサイズでメモリ確保
    data = new puyocolor[line*column];
    data_line = line;
    data_column = column;
  }

  //盤面の行数を返す
  unsigned int GetLine()
  {
    return data_line;
  }

  //盤面の列数を返す
  unsigned int GetColumn()
  {
    return data_column;
  }

  //盤面の指定された位置の値を返す
  puyocolor GetValue(unsigned int y, unsigned int x)
  {
    if (y >= GetLine() || x >= GetColumn())
      {
	//引数の値が正しくない
	return NONE;
      }

    return data[y*GetColumn() + x];
  }
  //盤面の指定された位置に値を書き込む
  void SetValue(unsigned int y, unsigned int x, puyocolor value)
  {
    if (y >= GetLine() || x >= GetColumn())
      {
	//引数の値が正しくない
	return;
      }

    data[y*GetColumn() + x] = value;
  }

};

//落下中
class PuyoArrayActive : public PuyoArray
{
public:
  int puyorotate;
  PuyoArrayActive(){puyorotate = 0;}
  
};


//着地済み
class PuyoArrayStack : public PuyoArray
{

};

//得点処理
class PuyoArrayScore
{
public:
  int vanish; //消えたぷよ数
  int chain; //連鎖数
  int connect; //連結数
  int score; //スコア

  //初期化
  PuyoArrayScore(){vanish = 0, chain=0, connect = 0, score = 0;}

  //連鎖数のカウントと連結ボーナスの計算
  void AddScore(int v){
    if (v != 0){
      chain++;
      if (v >= 11){
	connect += 10;
      }
      else if (v == 4){}
      else{
	connect += v-3;
      }
    }
  }

  //連鎖ボーナスの計算とボーナスの合計の計算
  int TotalAddScore(){
    int cslist[] = {0, 0, 8, 16, 32, 64, 96, 128, 160, 192,
		    224, 256, 288, 320, 352, 384, 416, 448, 480, 512};

    int sbscore = 0;

    sbscore += connect;
    
    if (chain < 20)
      {
        sbscore += cslist[chain];
      }else{
	sbscore += 700;
      }

    //ボーナスが0の時は1を返す
    if (sbscore == 0)
      {
	sbscore = 1;
      }

    return sbscore;
  }

  //スコアを計算
  int TotalScore(){
    if(vanish != 0){
      score += vanish*10*TotalAddScore();
      vanish = 0;
    }
   return score;
  }
  
};

class PuyoControl
{
public:  
  //盤面に新しいぷよ生成
  void GeneratePuyo(PuyoArrayActive &puyo)
  {
    puyo.puyorotate = 0;
      
    srand(time(NULL));
    int rndcol1 = rand() % 4 + 1;
    int rndcol2 = rand() % 4 + 1;
    int rndint = rand() % (puyo.GetColumn()-1);

    puyocolor newpuyo1;
    newpuyo1 = puyocolor(rndcol1);

    puyocolor newpuyo2;
    newpuyo2 = puyocolor(rndcol2);

    puyo.SetValue(0, rndint, newpuyo1);
    puyo.SetValue(0, rndint+1, newpuyo2);
  }

  //ぷよの着地判定．着地判定があるとtrueを返す
  bool LandingPuyo(PuyoArrayActive &puyo, PuyoArrayStack &puyostack)
  {
   bool landed = false;

   for (int y = 0; y < puyo.GetLine(); y++)
     {
       for (int x = 0; x < puyo.GetColumn(); x++)
	 {
	   //横並びで一番下に落ちたぷよ
	   if (puyo.GetValue(y, x) != NONE && y == puyo.GetLine() - 1)
	     {
		 landed = true;

		 puyostack.SetValue(y, x, puyo.GetValue(y,x));
		 puyo.SetValue(y, x, NONE);
	       }
	   //縦並びで一番下に落ちたぷよ
	   else if (puyo.GetValue(y,x) != NONE && y == puyo.GetLine()-2 && puyo.GetValue(y+1,x) != NONE)
	     {
	       landed = true;

	       puyostack.SetValue(y, x, puyo.GetValue(y,x));
	       puyo.SetValue(y, x, NONE);
	       puyostack.SetValue(y+1, x, puyo.GetValue(y+1, x));
	       puyo.SetValue(y+1, x, NONE);
	     }
	   //着地済みぷよの上に縦並びで落ちたぷよ
	   else if(puyo.GetValue(y,x) != NONE && puyostack.GetValue(y+2, x) != NONE && puyo.GetValue(y+1,x) != NONE)
	     {
	       landed = true;

	       puyostack.SetValue(y, x, puyo.GetValue(y,x));
	       puyo.SetValue(y, x, NONE);
	       puyostack.SetValue(y+1, x, puyo.GetValue(y+1, x));
	       puyo.SetValue(y+1, x, NONE);
	     }
	   //着地済みぷよの上に横並びで落ちたぷよ
	   else if (puyo.GetValue(y, x) != NONE && puyostack.GetValue(y+1, x) != NONE)
	       {
	         landed = true;

		 puyostack.SetValue(y, x, puyo.GetValue(y,x));
		 puyo.SetValue(y, x, NONE);
		 
		 //着地判定を受けたぷよの隣も確認する
	         if (puyo.GetValue(y, x+1) != NONE)
	          {
	            puyostack.SetValue(y, x+1, puyo.GetValue(y, x+1));
	            puyo.SetValue(y, x+1, NONE);
		    for (int i=0; i<puyostack.GetLine(); i++){
		      if(puyostack.GetValue(y+i+1, x+1) == NONE && y+i+1 != puyostack.GetLine())
		        {
			  puyostack.SetValue(y+i+1, x+1, puyostack.GetValue(y+i, x+1));
		          puyostack.SetValue(y+i, x+1, NONE);
		      }
		    }
		  }
	         else if (puyo.GetValue(y, x-1) != NONE)
	          {
	            puyostack.SetValue(y, x-1, puyo.GetValue(y, x-1));
	            puyo.SetValue(y, x-1,NONE);
		    for (int i=0; i<puyostack.GetLine(); i++){
		      if(puyostack.GetValue(y+i+1, x-1) == NONE && y+i+1 != puyostack.GetLine())
			{
			  puyostack.SetValue(y+i+1, x-1, puyostack.GetValue(y+i, x-1));
			  puyostack.SetValue(y+i, x-1, NONE);
			}
		    }
		    
		  }
	       }
	 }
     }
   return landed;
  }


  //左移動
  void MoveLeft(PuyoArrayActive &puyo, PuyoArrayStack &puyostack)
  {
   //一時的格納場所メモリ確保
   puyocolor *puyo_temp = new puyocolor[puyo.GetLine()*puyo.GetColumn()];

   for (int i = 0; i < puyo.GetLine()*puyo.GetColumn(); i++)
     {
       puyo_temp[i] = NONE;
     }
   //1つ左の位置にpuyoactiveからpuyo_tempへとコピー
   for (int y = 0; y < puyo.GetLine(); y++)
     {
       for (int x = 0; x < puyo.GetColumn(); x++)
	 {
	   if (puyo.GetValue(y, x) == NONE) {
	     continue;
	   }
	   if (0 < x && puyo.GetValue(y, x-1) == NONE && puyostack.GetValue(y, x-1) == NONE)
	     {
	       //縦並びのぷよで左に動かせる時
	       if (puyo.GetValue(y+1,x) != NONE && puyostack.GetValue(y+1, x-1) == NONE)
		 {
		   puyo_temp[y*puyo.GetColumn() + (x-1)] = puyo.GetValue(y,x);
		   puyo.SetValue(y, x, NONE);
	           puyo_temp[(y+1)*puyo.GetColumn() + (x-1)] = puyo.GetValue(y+1,x);
       	           puyo.SetValue(y+1, x, NONE);
		 }
	       //縦並びのぷよで左に動かせない時
	       else if (puyo.GetValue(y+1, x) != NONE && puyostack.GetValue(y+1,x-1) != NONE)
		 {
		   puyo_temp[y*puyo.GetColumn() + x] = puyo.GetValue(y,x);
		   puyo_temp[(y+1)*puyo.GetColumn() + x] = puyo.GetValue(y+1, x);
		 }
	       //横並びのぷよで左に動かせる時
	       else
	        {
		  puyo_temp[y*puyo.GetColumn() + (x - 1)] = puyo.GetValue(y, x);
	          //コピー後に元位置のpuyoactiveのデータは消す
	          puyo.SetValue(y, x, NONE);
		}
	     }
	   
	   else
	     {
	       puyo_temp[y*puyo.GetColumn() + x] = puyo.GetValue(y, x);
	     }
	 }
     }

   //puyo_tempからpuyoactiveへコピー
   for (int y = 0; y < puyo.GetLine(); y++)
     {
       for (int x = 0; x < puyo.GetColumn(); x++)
	 {
	   puyo.SetValue(y, x, puyo_temp[y*puyo.GetColumn() + x]);
	 }
     }
   //一時的格納場所メモリ解放
   delete[] puyo_temp;
  }

  //右移動
  void MoveRight(PuyoArrayActive &puyo, PuyoArrayStack &puyostack)
  {
   //一時的格納場所メモリ確保
   puyocolor *puyo_temp = new puyocolor[puyo.GetLine()*puyo.GetColumn()];

   for (int i = 0; i < puyo.GetLine()*puyo.GetColumn(); i++)
     {
       puyo_temp[i] = NONE;
     }

   //1つ右の位置にpuyoactiveからpuyo_tempへとコピー
   for (int y = 0; y < puyo.GetLine(); y++)
     {
       for (int x = puyo.GetColumn()-1; x >= 0; x--)
	 {
	   if (puyo.GetValue(y, x) == NONE) {
	     continue;
	   }
	   if (x < puyo.GetColumn()-1 && puyo.GetValue(y, x+1) == NONE && puyostack.GetValue(y, x+1) == NONE)
	     {
	       //縦並びのぷよで右に移動できる時
	       if (puyo.GetValue(y+1, x) != NONE && puyostack.GetValue(y+1, x+1) == NONE)
		 {
		   puyo_temp[y*puyo.GetColumn() + (x+1)] = puyo.GetValue(y, x);
		   puyo.SetValue(y, x, NONE);
		   puyo_temp[(y+1)*puyo.GetColumn() + (x+1)] = puyo.GetValue(y+1, x);
		   puyo.SetValue((y+1), x, NONE);
		 }
	       //縦並びのぷよで右に移動できない時
	       else if (puyo.GetValue(y+1,x) != NONE && puyostack.GetValue(y+1, x+1) != NONE)
		 {
		   puyo_temp[y*puyo.GetColumn() + x] = puyo.GetValue(y,x);
		   puyo.SetValue(y, x, NONE);
		   puyo_temp[(y+1)*puyo.GetColumn() + x] = puyo.GetValue(y,x);
		   puyo.SetValue(y, x, NONE);
		 }
	       //横並びのぷよで右に移動できる時
	       else
		 {
		   puyo_temp[y*puyo.GetColumn() + (x+1)] = puyo.GetValue(y, x);
	           //コピー後に元位置のpuyoactiveのデータは消す
	           puyo.SetValue(y, x, NONE);
	         }
	     }
	   else
	     {
	       puyo_temp[y*puyo.GetColumn() + x] = puyo.GetValue(y, x);
	     }
	 }
     }

   //puyo_tempからpuyoactiveへコピー
   for (int y = 0; y <puyo.GetLine(); y++)
     {
       for (int x = 0; x <puyo.GetColumn(); x++)
	 {
	   puyo.SetValue(y, x, puyo_temp[y*puyo.GetColumn() + x]);
	 }
     }

   //一時的格納場所メモリ解放
   delete[] puyo_temp;
  }

  //下移動
  void MoveDown(PuyoArrayActive &puyo, PuyoArrayStack &puyostack)
  {
   //一時的格納場所メモリ確保
   puyocolor *puyo_temp = new puyocolor[puyo.GetLine()*puyo.GetColumn()];
   
   for (int i = 0; i < puyo.GetLine()*puyo.GetColumn(); i++)
     {
       puyo_temp[i] = NONE;
     }

   //1つ下の位置にpuyoactiveからpuyo_tempへとコピー
   for (int y = puyo.GetLine() - 1; y >= 0; y--)
     {
       for (int x = 0; x < puyo.GetColumn(); x++)
	 {
	   if (puyo.GetValue(y, x) == NONE) {
	     continue;
	   }

	   if (y < puyo.GetLine() - 1 && puyo.GetValue(y + 1, x) == NONE && puyostack.GetValue(y+1,x) == NONE && (puyo.GetValue(y, x+1) == NONE || puyostack.GetValue(y+1, x+1)== NONE) && (puyo.GetValue(y, x-1) == NONE || puyostack.GetValue(y+1, x-1) == NONE))
	     {
	       	puyo_temp[(y + 1)*puyo.GetColumn() + x] = puyo.GetValue(y, x);
		//コピー後に元位置のpuyoactiveのデータは消す
		puyo.SetValue(y, x, NONE);
	     }
	   else
	     {
	       puyo_temp[y*puyo.GetColumn() + x] = puyo.GetValue(y, x);
	     }
	 }
     }

   //puyo_tempからpuyoactiveへコピー
   for (int y = 0; y < puyo.GetLine(); y++)
     {
       for (int x = 0; x < puyo.GetColumn(); x++)
	 {
	   puyo.SetValue(y, x, puyo_temp[y*puyo.GetColumn() + x]);
	 }
     }

   //一時的格納場所メモリ解放
   delete[] puyo_temp;
  }


  //ぷよの回転
  void Rotate(PuyoArrayActive &puyo, PuyoArrayStack &puyostack)
  {
    puyocolor puyo1, puyo2;
    int puyo1_x = 0;
    int puyo1_y = 0;
    int puyo2_x = 0;
    int puyo2_y = 0;

    bool findingpuyo1 = true;
    for (int y =0; y<puyo.GetLine(); y++)
      {
	for (int x=0; x<puyo.GetColumn(); x++)
	  {
	    if (puyo.GetValue(y,x) != NONE)
	      {
		if(findingpuyo1)
		  {
		    puyo1 = puyo.GetValue(y,x);
		    puyo1_x = x;
		    puyo1_y = y;
		    findingpuyo1 = false;
		  }
		else
		  {
		    puyo2 = puyo.GetValue(y,x);
		    puyo2_x = x;
		    puyo2_y = y;
		  }
	      }
	  }
      }

    //回転前のぷよを消す
    puyo.SetValue(puyo1_y, puyo1_x, NONE);
    puyo.SetValue(puyo2_y, puyo2_x, NONE);

    //操作中ぷよの回転
    switch (puyo.puyorotate)
      {
      case 0:
	//RB -> R
	//      B
	//Rがpuyo1, Bがpuyo2
	if(puyo2_x <= 0 || puyo2_y >= puyo.GetLine()-1 || puyostack.GetValue(puyo2_y + 1, puyo2_x) != NONE || puyostack.GetValue(puyo1_y + 1, puyo1_x) != NONE)  //回転した結果field_arrayの範囲外にでるなら回転しない、回転先にぷよがあるなら回転しない
	  {
	    puyo.SetValue(puyo1_y, puyo1_x, puyo1);
	    puyo.SetValue(puyo2_y, puyo2_x, puyo2);
	    break;
	  }

	//回転後の位置にぷよを置く
	puyo.SetValue(puyo1_y, puyo1_x, puyo1);
	puyo.SetValue(puyo2_y + 1, puyo2_x - 1, puyo2);
	//次の回転パターンの設定
	puyo.puyorotate = 1;
	break;

      case 1:
	//R -> BR
	//B
	//Rがpuyo1, Bがpuyo2
	if(puyo2_x <= 0 || puyo2_y <= 0 || puyostack.GetValue(puyo2_y, puyo2_x - 1) != NONE || puyostack.GetValue(puyo1_y, puyo1_x - 1) != NONE) //回転した結果field_arrayの範囲外にでるなら回転しない、回転先にぷよがあるなら回転しない
	  {
	    puyo.SetValue(puyo1_y, puyo1_x, puyo1);
	    puyo.SetValue(puyo2_y, puyo2_x, puyo2);
	    break;
	  }

	//回転後の位置にぷよを置く
	puyo.SetValue(puyo1_y, puyo1_x, puyo1);
	puyo.SetValue(puyo2_y - 1, puyo2_x - 1, puyo2);
	//次の回転パターンの設定
	puyo.puyorotate = 2;
	break;

      case 2:
	//      B
	//BR -> R
	//Bがpuyo1, Rがpuyo2
	if(puyo1_x >= puyo.GetColumn()-1 || puyo1_y <= 0 || puyostack.GetValue(puyo1_y - 1, puyo1_x) != NONE || puyostack.GetValue(puyo2_y - 1, puyo2_x) != NONE) //回転した結果field_arrayの範囲外にでるなら回転しない、回転先にぷよがあるなら回転しない
	  {
	    puyo.SetValue(puyo1_y, puyo1_x, puyo1);
	    puyo.SetValue(puyo2_y, puyo2_x, puyo2);
	    break;
          }

	//回転後の位置にぷよを置く
	puyo.SetValue(puyo1_y - 1, puyo1_x + 1, puyo1);
	puyo.SetValue(puyo2_y, puyo2_x, puyo2);
	//次の回転パターンの設定
	puyo.puyorotate = 3;
	break;

      case 3:
	//B
	//R -> RB
	//Bがpuyo1, Rがpuyo2
	if(puyo1_x >= puyo.GetColumn() - 1 || puyo1_y >= puyo.GetLine() - 1 || puyostack.GetValue(puyo1_y, puyo1_x + 1) != NONE || puyostack.GetValue(puyo2_y, puyo2_x + 1) != NONE) //回転した結果field_arrayの範囲外にでるなら回転しない、回転先にぷよがあるなら回転しない
	  {
	    puyo.SetValue(puyo1_y, puyo1_x, puyo1);
	    puyo.SetValue(puyo2_y, puyo2_x, puyo2);
	    break;
	  }

	//回転後の位置にぷよを置く
	puyo.SetValue(puyo1_y + 1, puyo1_x + 1, puyo1);
	puyo.SetValue(puyo2_y, puyo2_x, puyo2);
	//次の回転パターンの設定
	puyo.puyorotate = 0;
	break;

      default:
	break;
      }
  }


  //ぷよ消滅処理を全座標で行う
  //
  //消滅したぷよの数を返す
  int VanishPuyo(PuyoArrayStack &puyostack, PuyoArrayScore &puyoscore)
  {
    int vanishednumber = 0;
    int v = 0;

    //各ボーナス得点を入れる変数を初期化
    puyoscore.chain = 0;
    puyoscore.connect = 0;
    
    for (int y=0; y<puyostack.GetLine(); y++)
      {
	for (int x=0; x<puyostack.GetColumn(); x++)
	  {
	    v = VanishPuyo(puyostack, y, x);
	    vanishednumber += v;

	    //ボーナス得点計算処理
	    puyoscore.AddScore(v);
	  }
      }
  
    puyoscore.vanish = vanishednumber;
    return vanishednumber;
  }
  
  //ぷよ消滅処理を座標で行う
  //消滅したぷよの数を返す
  int VanishPuyo(PuyoArrayStack &puyostack, unsigned int y, unsigned int x)
  {
    //判定箇所にぷよがなければ処理終了
    if (puyostack.GetValue(y,x) == NONE)
      {
	return 0;
      }

    //判定状態を表す列挙型
    //NOCHECK判定未実施、CHECKING判定対象、CHECKED判定済み
    enum checkstate{ NOCHECK, CHECKING, CHECKED };

    //判定結果格納用の配列
    enum  checkstate *field_array_check;
    field_array_check = new enum checkstate[puyostack.GetLine()*puyostack.GetColumn()];

    //配列初期化
    for (int i=0; i<puyostack.GetLine()*puyostack.GetColumn(); i++)
      {
	field_array_check[i] = NOCHECK;
      }

    //座標(x,y)を判定対象にする
    field_array_check[y*puyostack.GetColumn() + x] = CHECKING;

    //判定対象が１つもなくなるまで、判定対象の上下左右に同じ色のぷよがあるか確認し、あれば新たな判定対象にする
    bool checkagain = true;
    while (checkagain)
      {
	checkagain = false;

	for(int y=0; y<puyostack.GetLine(); y++)
	  {
	    for(int x=0; x<puyostack.GetColumn(); x++)
	      {
	        //(x,y)に判定対象がある場合
		if (field_array_check[y*puyostack.GetColumn() + x] == CHECKING)
		  {
		    //(x+1,y)の判定
		    if (x < puyostack.GetColumn()-1)
		      {
			//(x+1,y)と(x,y)のぷよの色が同じで、(x+1,y)のぷよが判定未実施か確認
			if(puyostack.GetValue(y,x+1) == puyostack.GetValue(y,x) && field_array_check[y*puyostack.GetColumn() + (x+1)] == NOCHECK)
			  {
			    field_array_check[y*puyostack.GetColumn() + (x+1)] = CHECKING;
			    checkagain = true;
			  }
		      }
		    //(x-1,y)の判定
		    if (x > 0)
		      {
		        if(puyostack.GetValue(y,x-1) == puyostack.GetValue(y,x) && field_array_check[y*puyostack.GetColumn() + (x-1)] == NOCHECK)
		          {
			    field_array_check[y*puyostack.GetColumn() + (x-1)] = CHECKING;
			    checkagain = true;
		          }
		      }
		    //(x,y+1)の判定
		    if(y < puyostack.GetLine() -1)
		      {
		        if(puyostack.GetValue(y+1,x) == puyostack.GetValue(y,x) && field_array_check[(y+1)*puyostack.GetColumn() + x] == NOCHECK)
		          {
			    field_array_check[(y+1)*puyostack.GetColumn() + x] = CHECKING;
			    checkagain = true;
		          }
		      }
		    //(x,y-1)の判定
		    if(y > 0)
		      {
		        if(puyostack.GetValue(y-1,x) == puyostack.GetValue(y,x) && field_array_check[(y-1)*puyostack.GetColumn() + x] == NOCHECK)
		          {
			    field_array_check[(y-1)*puyostack.GetColumn() + x] = CHECKING;
			    checkagain = true;
		          }
		      }

		    //(x,y)を判定済みにする
		    field_array_check[y*puyostack.GetColumn() + x] = CHECKED;
		  }
	      }
	  }
      }

    //判定済みの数をカウント
    int puyocount = 0;
    for (int i=0; i<puyostack.GetLine()*puyostack.GetColumn(); i++)
      {
	if (field_array_check[i] == CHECKED)
	  {
	    puyocount++;
	  }
      }

    //4個以上あれば、判定済み座標のぷよを消す
    int vanishednumber = 0;
    
    if (4 <= puyocount)
      {
	for(int y=0; y<puyostack.GetLine(); y++)
	  {
	    for(int x=0; x<puyostack.GetColumn(); x++)
	      {
		if(field_array_check[y*puyostack.GetColumn() + x] == CHECKED)
		  {
		    puyostack.SetValue(y, x, NONE);
		    vanishednumber++;
	          }
	      }
	  }
      }

    //ぷよを消したことで浮くぷよがあれば落とす
    for (int y = 0; y < puyostack.GetLine(); y++)
      {
	for (int x = 0; x < puyostack.GetColumn(); x++)
	  {
	    if (puyostack.GetValue(y, x) != NONE && y != puyostack.GetLine() - 1 && puyostack.GetValue(y+1, x) == NONE)
	     {
	       for (int i=0; i< puyostack.GetLine(); i++)
		 {
		   if(puyostack.GetValue(y+i+1, x) == NONE && y+i+1 != puyostack.GetLine())
		     {
		       puyostack.SetValue(y+i+1, x, puyostack.GetValue(y+i,x));
		       puyostack.SetValue(y+i, x, NONE);
		     }
		 }
	     }
	  }
      }
    
    //メモリ解放
    delete[] field_array_check;

    return vanishednumber;
  }
          
};

//表示
void Display(PuyoArrayActive &puyo, PuyoArrayStack &puyostack, PuyoArrayScore &puyoscore)
{
	//落下中ぷよ表示
	for (int y = 0; y < puyo.GetLine(); y++)
	{
		for (int x = 0; x < puyo.GetColumn(); x++)
		{
		  switch (puyo.GetValue(y, x) + puyostack.GetValue(y, x))
			{
			case NONE:
			        if (y == 2){
				  attrset(COLOR_PAIR(1));
			          mvaddch(y, x, '_');
			          break;}
			        else{
			          attrset(COLOR_PAIR(0));
			          mvaddch(y, x, '.');
			          break;}
      			case RED:
			        attrset(COLOR_PAIR(1));
			        mvaddch(y, x, 'R');
			        break;
			case BLUE:
			        attrset(COLOR_PAIR(2));
				mvaddch(y, x, 'B');
				break;
			case GREEN:
			        attrset(COLOR_PAIR(3));
				mvaddch(y, x, 'G');
				break;
			case YELLOW:
			        attrset(COLOR_PAIR(4));
				mvaddch(y, x, 'Y');
				break;
			default:
				mvaddch(y, x, '?');
				break;
			}
		}
	}

	
	//情報表示
	int count = 0;
	for (int y = 0; y < puyo.GetLine(); y++)
	{
		for (int x = 0; x < puyo.GetColumn(); x++)
		{
		  if (puyo.GetValue(y, x) != NONE || puyostack.GetValue(y,x) != NONE)
			{
				count++;
			}
		}
	}

	char msg[256];
	sprintf(msg, "Field: %d x %d, Puyo number: %03d", puyo.GetLine(), puyo.GetColumn(), count);
	attrset(COLOR_PAIR(0));
	mvaddstr(2, COLS - 35, msg);

  
	sprintf(msg, "Score: %d", puyoscore.TotalScore());
	attrset(COLOR_PAIR(0));
        mvaddstr(3, COLS - 35, msg);

	
	refresh();
}


//ここから実行される
int main(int argc, char **argv){
	//クラスのインスタンスを作成
	PuyoArrayActive puyo;
	PuyoArrayStack puyostack;
	PuyoArrayScore puyoscore;
	PuyoControl control;			
				
	//画面の初期化
	initscr();
	
	//カラー属性を扱うための初期化
	start_color();
	init_pair(0, COLOR_WHITE, COLOR_BLACK);
	init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_BLUE, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);

	//キーを押しても画面に表示しない
	noecho();
	//キー入力を即座に受け付ける
	cbreak();

	curs_set(0);
	//キー入力受付方法指定
	keypad(stdscr, TRUE);

	//キー入力非ブロッキングモード
	timeout(0);


	//初期化処理
	puyo.ChangeSize(LINES/2, COLS/2);	//フィールドは画面サイズの縦横1/2にする
	puyostack.ChangeSize(LINES/2, COLS/2);
	control.GeneratePuyo(puyo);	//最初のぷよ生成

	int delay = 0;
	int waitCount = 20000;

	int puyostate = 0;

	int a = 0;
	
	//メイン処理ループ
	while (1)
	{
		//キー入力受付
		int ch;
		ch = getch();

		//Qの入力で終了
		if (ch == 'Q')
		{
			break;
		}

		//画面の最も上まで積みあがったら終了
		for (int x=0; x<puyostack.GetColumn(); x++)
		  {
		    if(puyostack.GetValue(2, x) != NONE)
		      {
			a = 1;
		      }
		  }
		if (a==1)
		  {
		    char msg[256];
		    attrset(COLOR_PAIR(1));
		    sprintf(msg, "-------------");
		    mvaddstr(LINES/4+1, 7*COLS/10, msg);
	            sprintf(msg, "| GAME OVER |");
	            mvaddstr(LINES/4, 7*COLS/10, msg);
		    sprintf(msg, "-------------");
		    mvaddstr(LINES/4-1, 7*COLS/10, msg);

		    refresh();
		    sleep(7);
		    break;
		  }

		//入力キーごとの処理
		switch (ch)
		{
		case KEY_LEFT:
		        control.MoveLeft(puyo, puyostack);
			break;
		case KEY_RIGHT:
		        control.MoveRight(puyo, puyostack);
			break;
		case 'z':
		  control.Rotate(puyo, puyostack);
			break;
		case KEY_DOWN:
		        delay = waitCount;
		        break;
		default:
			break;
		}


		//処理速度調整のためのif文
		if (delay%waitCount == 0){
			//ぷよ下に移動
		  control.MoveDown(puyo, puyostack);
			
			//ぷよ着地判定
			if (control.LandingPuyo(puyo, puyostack))
			{
			  //消せるぷよがあるか確認、あれば消す
			  control.VanishPuyo(puyostack, puyoscore);
			  //着地していたら新しいぷよ生成
			  control.GeneratePuyo(puyo);
			}
		}
		delay++;

	   //表示
		Display(puyo, puyostack, puyoscore);

	}


	//画面をリセット
	endwin();

	return 0;
}
