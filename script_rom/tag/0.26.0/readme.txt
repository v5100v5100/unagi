famicom ROM cartridge utility - unagi
ROM dump script version 0.26.0
公式サイト http://sourceforge.jp/projects/unagi/

--はじめに--
筆者に手元にあったカセットを読み出すために使用しました。補足説明が必要
な物の説明です。動作未確認と記載したもので動作確認がとれたものは是非報
告をお願いします。

----
diskbios.map
----
disksystem の bios 専用です。このスクリプトは、NESヘッダを生成しません。
client 0.5.0 + driver onajimi のみ動作確認。

----
mmc1_4M.map
----
大容量ROM搭載のファイナルファンタジーI.IIとドラゴンクエストIV専用です。
後者は動作未確認。

----
mmc1normal.map
----
上記以外の MMC1 搭載ソフトに使用してください。キャラクタROMが2Mのもの
が動作未確認なので動かないかもしれません。

----
namcot106.map
----
キングオブキングスのみ確認。このマッパを使用しているソフトは結構多く、
キャラクタRAMを使用しているものがあるようですが、動作未確認です。
namcot カートリッジのデータ読み出しがクライアント側の問題で安定しない
ようです。

----
rc809.map
----
RC809と書かれたグーニーズ初代専用です。グーニーズはバリエーションが異
様に豊富なのでお間違えなきよう。

----
sunsoft3.map
----
コメントにも書いたんですが、スクリプトを書き終わった後にこのマッパのカ
セットを持ってないことに気づきました。動作未確認です。

----
sunsoft4.map
----
$6000-$7fff を ROM にするソフト(バットマン??)があるようですが、それが
動くかわかりません。

----
vrc7.map
----
ラグランジュポイントのみ動作確認済み。キャラクタROM搭載の Tiny Toon 
Adventures 2 動作未確認。

--Konami VRC series--
iNES の定義は結果としてかなりずさんで、マッパ番号はあてになりません。
ゲームタイトルと一致するスクリプトを使用してください。
vrc シリーズを採用していないものは無関係です。

vrc1.map:
	がんばれゴエモン！からくり道中
	キングコング2 怒りのメガトンパンチ
	エキサイティングボクシング
	鉄腕アトム
vrc2a01.map:
	コナミ ワイワイワールド
	月風魔伝
	ドラゴンスクロール 甦りし魔竜 (未確認)
	魂斗羅 (注:初代)
	じゃりん子チエ
	がんばれゴエモン2
vrc2a10.map:
	がんばれペナントレース
	ツインビー3 ポコポコ大魔王
vrc3.map:
	沙羅曼蛇
vrc4a.map:
	ワイワイワールド2 SOS！！パセリ城
vrc4b.map:
	バイオミラクル ぼくってウパ (注:ROM)
	がんばれゴエモン外伝 消えた黄金キセル
	グラディウスII
	レーサーミニ四駆 ジャパンカップ
vrc4c.map:
	がんばれゴエモン外伝2 天下の財宝
vrc4d.map:
	Teenage Mutant Ninja Turtles
	Teenage Mutant Ninja Turtles 2
vrc4e.map:
	パロディウスだ！
	悪魔城すぺしゃる ぼくドラキュラくん
	クライシスフォース
	タイニートゥーン・アドベンチャーズ (注:初代)
vrc6a01.map:
	悪魔城伝説 (未確認, 未作成)
vrc6a10.map:
	魍魎戦記 MADARA
	エスパードリーム2
vrc7.map:
	ラグランジュポイント
	タイニートゥーン・アドベンチャーズ2 (動作未確認で別スクリプトの可能性大)

--スクリプト募集--
150くらいあるマッパのスクリプトを全て私だけで書くことは出来ません。手
軽にスクリプトを追加することが出来るので、スクリプトが書けた人は公式サ
イトまで連絡をください。
採用基準は下記とさせていただきます。

* そのスクリプトを使って実際に動作確認をして読み出せたこと(とかいって
  おきながら sunsoft3.map は動作未確認...)
* 動作確認をしたソフト数本の名称か型番をスクリプト先頭にコメントをいれ
  ること
* 読み出しに必要なレジスタは初期化してから読み出すこと (マッパのマイ
  ナーバージョンによって電源投入後の初期値が異なる場合があるので必ず
  行ってください)

