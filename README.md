# [Project Mulus](https://github.com/kahnn/Mulus)  2013/04/02

## 1. プロジェクト概要

Project Mulus (以降 Mulus) は、[エコーネットコンソーシアム](http://www.echonet.gr.jp/)による[ECHONET Lite 規格 Ver1.01](http://www.echonet.gr.jp/spec/spec_v101_lite.htm)をサポートする機器実現のための、C言語で書かれた ToolKit です。
元々は個人的な家電Hackのために始めましたが、今後普及が進むと考えられる HEMS において、自家製家電を組み入れたり、自分の好みで市販家電の制御を行ったりしたい愛好家の手助けになることを目指しています。


## 2. 機能一覧＆サポート状況

まだ、Mulus は開始されたばかりです。(2013/02/02現在 ALPHA-1リリース)
ECHONET Lite規格への準拠もまだ不充分です。

### **各種制限について**

```
---------------------------------------------------------
- 動作環境

現状では以下の環境での動作を確認している

 -- CentOS 6.4 (X86_64)

---------------------------------------------------------
- ECHONET Lite サポート仕様制限

SetGetについてはエラー（返却なし）とする
インスタンスコードが複数指定(inc==0x0)の際の処理が無い。
EPR処理で個数制限がある場合の処理が無い。
他ノードの情報(proxy)を保持することはできない。必要があれば、自分で処理する。

---------------------------------------------------------
- Mulus実装制限事項

エラーメッセージなどはstderrに直接出力される。
アーキテクチャとしてMultiThreadには対応していない。
シングルスレッドモデルなので、タイマー処理の中や、各プロパティ処理の中で長時間処理をおこなったりsleepしたりしないこと。
タイマー処理間隔は10秒固定だが、他の処理などにも影響され、必ずしも正確な時間にはならない。したがって、必要に応じてハンドラ中で時間の確認を行う事。
```

### **サポートオブジェクトについて**

```
機器オブジェクトスーパークラス (OBJD P.2-1)
温度センサクラス  (OBJD P.3-27)
```


## 3. 利用方法

Mulus の利用者は、動作対象の機器に応じて、Mulus のコード全てをそのまま使うのではなく、目的に応じて必要なコードを選択し使用することができます。むしろ、Mulus を部品群として、必要に応じてパーツ取りして役立ててもらえればと思います。
開発・動作確認は、Linux(2.6.32) 上でおこなっていますが、Linux固有のシステムコール等は基本的に使用しないようにしていますので、UNIX系のOS上であれば修正は軽微だと思います。

参考までに、主なソース構成を以下に示します。
なお、温度センサー サンプル および 各コマンドの仕様については、ソースおよび各ディレクトリのREADMEファイルを参照してください。

```
 TBD

    middleware       ... ECHONET Lite 実装用ミドルウェアディレクトリ
      comm           ... ECHONET Liteプロトコルサポート
      util           ... 汎用ライブラリ
      test           ... 汎用ライブラリテスト用
      object         ... 現在は空、将来は各種デバイス用のテンプレート＆ツール
      include        ... ミドルウェアが提供するインクルードファイル
      lib            ... ミドルウェアが提供するライブラリ

    tool             ... ツール用ディレクトリ
      /el-discover   ... インスタンスを保持するノードを検索する
      /el-prop       ... プロパティの参照・更新をおこなう
      /dev-config    ... ECHONET Liteプロトコルを使用しない管理コマンド
      /open-echo     ... リファレンス用のOpenECHOプログラム(*1)

    sample           ... サンプルプログラム用ディレクトリ
      /temp_sensor   ... 温度計センサー例

    doc              ... ドキュメント用ディレクトリ
```
 (*1) OpenECHO: https://github.com/SonyCSL/OpenECHO


## 4. 機器オブジェクトの追加

Mulus で用意されていない機器オブジェクトを追加する場合は、以下のようにおこないます。

```
 TBD
  - 該当機器オブジェクト用のソース[\*.hc]を作成
    ** テンプレートを追加すれば、そこから生成できるように整備する **
  - 固有プロパティの追加
  - ノードへの追加
  - プロファイルへの追加
  - イベントハンドリング
```


## 5. FAQ

* CentOS 6 でマルチキャストアドレスのパケットが受信できない
  127.0.0.1 にバインドすれば上手くいく場合は、Firewall ではじかれている可能性が高い。
  iptablesの設定を変更して、マルチキャストパケットが通るようにする。
  以下にデフォルトから修正した場合の概要を示す。(実際には、もっと対象を絞った方が良い)

```
# /etc/sysconfig/iptables の内容
------------------------------
# Firewall configuration written by system-config-firewall
# Manual customization of this file is not recommended.
*filter
:INPUT ACCEPT [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
-A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
-A INPUT -p icmp -j ACCEPT
-A INPUT -i lo -j ACCEPT
-A INPUT -m state --state NEW -m tcp -p tcp --dport 22 -j ACCEPT
##### ADD START HERE
-A INPUT -m pkttype --pkt-type multicast -j ACCEPT
-A INPUT --protocol igmp -j ACCEPT
## needed for unicast 
-A INPUT -p udp -j ACCEPT
-A OUTPUT -p udp -j ACCEPT
## needed for multicast ping responses
#-A INPUT -p icmp --icmp-type 0 -j ACCEPT
##### ADD END HERE
-A INPUT -j REJECT --reject-with icmp-host-prohibited
-A FORWARD -j REJECT --reject-with icmp-host-prohibited
COMMIT
------------------------------

# 設定の反映

# /etc/init.d/iptables restart
# iptables -L
```


## 6. Author

**Kahnn Ikeda**
* https://github.com/kahnn


## 7. Copyright and license

Mulus is free software, available under the terms of a [MIT license](http://opensource.org/licenses/mit-license.php).
Please see the LICENSE file for details.

## 8. リリース

- 2013/02/02  ALPHA-1 リリース
  - 基本機能の実装
- 2013/06/中  ALPHA-2 リリース予定
  - el-discover 複数ノード取得, el-prop 複数プロパティ対応などのtool拡張
  - ECHONET Liteオプション機能への対応
  - その他機能追加
- 2013/06/末  ALPHA-3 リリース予定
  - IPv6対応
  - その他機能追加
- 2013/07/中  BEAT-1  リリース予定
  - 内部設計の見直し、ECHONET Liteデバイス実装向けAPI I/Fの確定
- 2013/08/初  BEAT-2  リリース予定
  - デバイス実装用テンプレートの用意
- 2013/08/末  Ver.1.0 リリース予定
