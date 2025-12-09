#include "Transform.h"

const MATRIX& Transform::MakeLocalMatrix()
{
    // TODO: return ステートメントをここに挿入します

    MATRIX ms = MGetScale(scale);        //拡大縮小行列 scale は VECTOR 型（x, y, z）で、各軸方向の大きさを決めます。
                                         //例：scale = { 2, 1, 1 } → X方向に2倍のサイズになります。
    MATRIX mrx = MGetRotX(rotation.x);   //回転行列　 x軸回転の角度（ラジアン）
    MATRIX mry = MGetRotY(rotation.y);   //           y軸
    MATRIX mrz = MGetRotZ(rotation.z);   //           z軸
    MATRIX mt = MGetTranslate(position); //平行移動（位置）を行う行列
                                         //positionは物体のワールド上の位置ベクトル

    rotMatrix = ms * mrz * mrx * mry;    //これは、スケーリング → Z軸回転 → X軸回転 → Y軸回転、という順に「変換」を適用しています。
                                         //    順番に注意：行列の掛け算は順番が大事！
    localMatrix = rotMatrix * mt;        //ｍｔをかけてるから最後に平行移動（位置の設定）
    return localMatrix;
}
