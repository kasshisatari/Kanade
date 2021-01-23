/* Copyright 2020 oscillo

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software
and associated documentation files(the "Software"),
to deal in the Software without restriction,
including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense,
and / or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so,
subject to the following conditions :

The above copyright notice and this permission
notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY
OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

var userId = 0;
var ssid = "";
var pass = "";
var fileId = 0;
var bookId = 0;
var playListTimer = null;
var remoconTimer = null;
var historyId = 0;
var historyListJson = null;
var language = null;
var latestBookId = 0;
var latestBookFlag = false;
var latestUserId = 0;
var aPEnable = false;
var aPPass = "";
var audioPort = 0;
var locale = {
  "jp": {
    "loginNameLabel": "名前",
    "loginButton": "ログイン",
    "adminButton": "管理画面",
    "attentionButton": "注意事項",
    "loginAlert": "ログイン名が未入力です。\n匿名でログインします。",
    "resetHistoryButton": "履歴初期化",
    "refreshSongButton": "曲情報更新",
    "shutdownButton": "シャットダウン",
    "rebootButton": "再起動",
    "adminBackButton": "戻る",
    "adminTitleLabel": "管理画面",
    "attentionTitleLabel": "注意事項",
    "attentionLabel": "カラオケ店等にてコンセントを繋いで電源を使用する場合、店側の許可を得てください。刑法245条に抵触する『電気窃盗罪』が成立します。\n\n再生する動画像は営利目的への利用、インターネットに公開する、公衆の場での利用等、著作権法第38条第1項に抵触する場合、権利者の許諾を得てください。\n\nKanadeはMIT Licenseです。 以下に定める条件に従い、本ソフトウェアおよび関連文書のファイル（以下「ソフトウェア」）の複製を取得するすべての人に対し、ソフトウェアを無制限に扱うことを無償で許可します。これには、ソフトウェアの複製を使用、複写、変更、結合、掲載、頒布、サブライセンス、および/または販売する権利、およびソフトウェアを提供する相手に同じことを許可する権利も無制限に含まれます。\n\n上記の著作権表示および本許諾表示を、ソフトウェアのすべての複製または重要な部分に記載するものとします。\n\nソフトウェアは「現状のまま」で、明示であるか暗黙であるかを問わず、何らの保証もなく提供されます。ここでいう保証とは、商品性、特定の目的への適合性、および権利非侵害についての保証も含みますが、それに限定されるものではありません。 作者または著作権者は、契約行為、不法行為、またはそれ以外であろうと、ソフトウェアに起因または関連し、あるいはソフトウェアの使用またはその他の扱いによって生じる一切の請求、損害、その他の義務について何らの責任も負わないものとします。 ",
    "unableConnectAlert": "Kanadeに接続できません",
    "updateSongTitleLabel": "確認",
    "updateSongAlert": "ファイル情報を取り込みます。しばらく時間がかかる場合があります。よろしいでしょうか。",
    "updateSongYesButton": "はい",
    "updateSongNoButton": "いいえ",
    "shutdownConfirmTitleLabel": "確認",
    "shutdownConfirmAlert": "シャットダウンします。よろしいでしょうか。",
    "shutdownConfirmYesButton": "はい",
    "shutdownConfirmNoButton": "いいえ",
    "configAPButton": "AccessPoint設定",
    "resetHistoryConfirmTitleLabel": "確認",
    "resetHistoryConfirmAlert": "履歴を初期化します。よろしいでしょうか。",
    "resetHistoryConfirmYesButton": "はい",
    "resetHistoryConfirmNoButton": "いいえ",
    "rebootConfirmTitleLabel": "確認",
    "rebootConfirmAlert": "再起動します。よろしいでしょうか。",
    "rebootConfirmYesButton": "はい",
    "rebootConfirmNoButton": "いいえ",
    "aPConfigAlert": "Wi-Fiパスワードは8文字以上を指定してください。",
    "aPConfigRebootAlert": "再起動後に有効になります。",
    "searchCloseButton": "閉じる",
    "searchBackButton": "戻る",
    "searchRemoconButton": "リモコン",
    "searchPlayListButton": "予約状況",
    "searchHistoryButton": "履歴",
    "searchFavoriteButton": "お気に入り",
    "searchLogoutButton": "ログアウト",
    "searchKeywordLabel": "キーワード",
    "searchSortLabel": "ソート",
    "searchOrderByName": "名前順",
    "searchOrderByTime": "時刻順",
    "searchButton": "検索",
    "fileListCloseButton": "閉じる",
    "fileListBackButton": "戻る",
    "fileListRemoconButton": "リモコン",
    "fileListSearchButton": "検索",
    "fileListPlayListButton": "予約状況",
    "fileListHistoryButton": "履歴",
    "fileListFavoriteButton": "お気に入り",
    "fileListLogoutButton": "ログアウト",
    "result": "件",
    "fileListPrevButton": "前へ",
    "fileListNextButton": "次へ",
    "fileDetailCloseButton": "閉じる",
    "fileDetailBackButton": "戻る",
    "fileDetailRemoconButton": "リモコン",
    "fileDetailSearchButton": "検索",
    "fileDetailPlayListButton": "予約状況",
    "fileDetailHistoryButton": "履歴",
    "fileDetailFavoriteButton": "お気に入り",
    "fileDetailLogoutButton": "ログアウト",
    "addBookFileDetailButton": "予約",
    "insertBookFileDetailButton": "割り込み予約",
    "playCountUnit": "回",
    "addBookConfirmHeaderLabel": "確認",
    "addBookConfirmAddLabel": "予約しますか。",
    "addBookConfirmCommentLabel": "コメント",
    "secretAddBookCheckboxLabel": "非公開",
    "pauseAddBookCheckboxLabel": "演奏後に一時停止",
    "addBookConfirmYesButton": "はい",
    "addBookConfirmNoButton": "いいえ",
    "insertBookConfirmHeaderLabel": "確認",
    "insertBookConfirmInsertLabel": "割り込み予約しますか。",
    "insertBookConfirmCommentLabel": "コメント",
    "secretInsertBookCheckboxLabel": "非公開",
    "pauseInsertBookCheckboxLabel": "演奏後に一時停止",
    "insertBookConfirmYesButton": "はい",
    "insertBookConfirmNoButton": "いいえ",
    "remoconCloseButton": "閉じる",
    "remoconBackButton": "戻る",
    "remoconSearchButton": "検索",
    "remoconPlayListButton": "予約状況",
    "remoconHistoryButton": "履歴",
    "remoconFavoriteButton": "お気に入り",
    "remoconLogoutButton": "ログアウト",
    "downVolRemoconButton": "音量小",
    "upVolRemoconButton": "音量大",
    "rewindRemoconButton": "早戻し",
    "fastForwardRemoconButton": "早送り",
    "switchAudioRemoconButton": "音声切替",
    "pauseRemoconButton": "一時停止",
    "stopRemoconButton": "中止",
    "resumeButton": "再開",
    "noVideoState": "再生中の動画はありません。",
    "playVideoState": "再生中",
    "pauseVideoState": "一時停止中",
    "volStartLabel": "音量：",
    "volEndLabel": " %",
    "playListCloseButton": "閉じる",
    "playListBackButton": "戻る",
    "playListRemoconButton": "リモコン",
    "playListSearchButton": "検索",
    "playListHistoryButton": "履歴",
    "playListFavoriteButton": "お気に入り",
    "playListLogoutButton": "ログアウト",
    "playListHeaderLabel": "予約一覧",
    "restTimeHeadLabel": "残時間：",
    "finishTimeHeadLabel": "終了予定：",
    "addBreakButton": "休憩追加",
    "insertBreakButton": "休憩割り込み",
    "addBreakConfirmHeader": "確認",
    "addBreakConfirmLabel": "休憩を追加します。よろしいでしょうか。",
    "addBreakConfirmYesButton": "はい",
    "addBreakConfirmNoButton": "いいえ",
    "insertBreakConfirmHeader": "確認",
    "insertBreakConfirmLabel": "休憩を先頭に追加します。よろしいでしょうか。",
    "insertBreakConfirmYesButton": "はい",
    "insertBreakConfirmNoButton": "いいえ",
    "historyListCloseButton": "閉じる",
    "historyListBackButton": "戻る",
    "historyListRemoconButton": "リモコン",
    "historyListPlayListButton": "予約状況",
    "historyListSearchButton": "検索",
    "historyListFavoriteButton": "お気に入り",
    "historyListLogoutButton": "ログアウト",
    "historyListHeader": "履歴",
    "historyListPrevButton": "前へ",
    "historyListNextButton": "次へ",
    "historyDetailCloseButton": "閉じる",
    "historyDetailBackButton": "戻る",
    "historyDetailRemoconButton": "リモコン",
    "historyDetailPlayListButton": "予約状況",
    "historyDetailSearchButton": "検索",
    "historyDetailFavoriteButton": "お気に入り",
    "historyDetailLogoutButton": "ログアウト",
    "historyDetailHeader": "履歴詳細",
    "historyDetailCommentButton": "コメント変更",
    "historyDetailDeleteButton": "削除",
    "historyDetailCSVButton": "CSV",
    "modifyHistoryYesButton": "はい",
    "modifyHistoryNoButton": "いいえ",
    "modifyHistoryHeader": "確認",
    "modifyHistoryMessage": "この履歴のコメントを修正しますか。",
    "deleteHistoryHeader": "確認",
    "deleteHistoryMessage": "履歴を削除しますか。",
    "deleteHistoryYesButton": "はい",
    "deleteHistoryNoButton": "いいえ",
    "downloadHistoryHeader": "確認",
    "downloadHistoryMessage": "この履歴より後の履歴を一覧化して取得しますか。",
    "downloadHistoryYesButton": "はい",
    "downloadHistoryNoButton": "いいえ",
    "configAudioButton": "音声設定",
    "configPlayModeButton": "再生モード設定",
    "playModeConfigRequestLabel": "リクエスト再生モード",
    "playModeConfigShuffleLabel": "シャッフル再生モード",
    "playModeConfigTitleLabel": "再生モード設定",
    "playModeConfigBackButton": "戻る",
    "playModeConfigChangeButton": "設定",
    "audioConfigChangeButton": "設定",
    "audioConfigTitleLabel": "音声ポート設定",
    "audioConfigBackButton": "戻る",
    "audioConfigConfirmHeader": "確認",
    "audioConfigConfirmQuestion": "次の再生より音声出力ポートを変更しますか。",
    "audioConfigConfirmAttention": "次の動画再生から設定が有効になります。",
    "audioConfigConfirmYesButton": "はい",
    "audioConfigConfirmNoButton": "いいえ",
    "aPConfigTitleLabel": "AccessPoint設定",
    "aPConfigBackButton": "戻る",
    "aPConfigLabel": "8文字未満は無効(要再起動)",
    "aPConfigChangeButton": "設定",
    "aPConfigEnableLabel": "有効",
    "aPConfigDisableLabel": "無効",
    "stopConfirmHeader": "確認",
    "stopConfirmMessage": "動画再生を中止します。よろしいでしょうか。",
    "stopConfirmYesButton": "はい",
    "stopConfirmNoButton": "いいえ",
    "breakLabel": "休憩",
    "secretLabel": "非公開",
    "bookDetailHeader": "予約詳細",
    "bookDetailCloseButton": "閉じる",
    "bookDetailBackButton": "戻る",
    "bookDetailRemoconButton": "リモコン",
    "bookDetailSearchButton": "検索",
    "bookDetailPlayListButton": "予約状況",
    "bookDetailHistoryButton": "履歴",
    "bookDetailLogoutButton": "ログアウト",
    "bookDetailUpButton": "上に移動",
    "bookDetailRemoveButton": "削除",
    "bookDetailDownButton": "下に移動",
    "deleteBookConfirmHeader": "確認",
    "deleteBookConfirmMessage": "予約を削除します。よろしいでしょうか。",
    "deleteBookConfirmYesButton": "はい",
    "deleteBookConfirmNoButton": "いいえ"
  },
  "en": {
    "loginNameLabel": "Name",
    "loginButton": "Login",
    "adminButton": "Administrate",
    "attentionButton": "Attention",
    "loginAlert": "Name is empty.\nYou login Anonymously.",
    "resetHistoryButton": "Reset History",
    "refreshSongButton": "Refresh Songs",
    "shutdownButton": "Shutdown",
    "rebootButton": "Reboot",
    "adminBackButton": "Back",
    "adminTitleLabel": "Administrate",
    "attentionTitleLabel": "Attention",
    "attentionLabel": "Kanade is MIT License.\n\n Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: \n\n The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\n\n THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.",
    "unableConnectAlert": "Unable to connect to Kanade.",
    "updateSongTitleLabel": "CAUTION",
    "updateSongAlert": "Do you want to import songs? It may take a few time.",
    "updateSongYesButton": "Yes",
    "updateSongNoButton": "No",
    "shutdownConfirmTitleLabel": "CAUTION",
    "shutdownConfirmAlert": "Do you want to shutdown Kanade?",
    "shutdownConfirmYesButton": "Yes",
    "shutdownConfirmNoButton": "No",
    "configAPButton": "Access Point",
    "resetHistoryConfirmTitleLabel": "CAUTION",
    "resetHistoryConfirmAlert": "Do you want to delete all history?",
    "resetHistoryConfirmYesButton": "Yes",
    "resetHistoryConfirmNoButton": "No",
    "rebootConfirmTitleLabel": "CAUTION",
    "rebootConfirmAlert": "Do you want to reboot Kanade?",
    "rebootConfirmYesButton": "Yes",
    "rebootConfirmNoButton": "No",
    "aPConfigAlert": "The Wi-Fi pass phrase must be more than 8 characters.",
    "aPConfigRebootAlert": "You have to reboot for the Wi-Fi passphrase configuration.",
    "searchCloseButton": "Close",
    "searchBackButton": "Back",
    "searchRemoconButton": "Remote",
    "searchPlayListButton": "Playlist",
    "searchHistoryButton": "History",
    "searchFavoriteButton": "Favorite",
    "searchLogoutButton": "Logout",
    "searchKeywordLabel": "Keyword",
    "searchSortLabel": "Sort",
    "searchOrderByName": "Sorted by Name",
    "searchOrderByTime": "Sorted by Date and Time",
    "searchButton": "Search",
    "fileListCloseButton": "Close",
    "fileListBackButton": "Back",
    "fileListRemoconButton": "Remote",
    "fileListSearchButton": "Search",
    "fileListPlayListButton": "Playlist",
    "fileListHistoryButton": "History",
    "fileListFavoriteButton": "Favorite",
    "fileListLogoutButton": "Logout",
    "result": " results",
    "fileListPrevButton": "Prev",
    "fileListNextButton": "Next",
    "fileDetailCloseButton": "Close",
    "fileDetailBackButton": "Back",
    "fileDetailRemoconButton": "Remote",
    "fileDetailSearchButton": "Search",
    "fileDetailPlayListButton": "Playlist",
    "fileDetailHistoryButton": "History",
    "fileDetailFavoriteButton": "Favorite",
    "fileDetailLogoutButton": "Logout",
    "addBookFileDetailButton": "Add",
    "insertBookFileDetailButton": "Insert",
    "playCountUnit": " plays",
    "addBookConfirmHeaderLabel": "Confirm",
    "addBookConfirmAddLabel": "Do you add it to playlist?",
    "addBookConfirmCommentLabel": "Comment",
    "secretAddBookCheckboxLabel": "Secret",
    "pauseAddBookCheckboxLabel": "Pause after playing",
    "addBookConfirmYesButton": "Yes",
    "addBookConfirmNoButton": "No",
    "insertBookConfirmHeaderLabel": "Confirm",
    "insertBookConfirmInsertLabel": "Do you insert it to playlist?",
    "insertBookConfirmCommentLabel": "Comment",
    "secretInsertBookCheckboxLabel": "Secret",
    "pauseInsertBookCheckboxLabel": "Pause after playing",
    "insertBookConfirmYesButton": "Yes",
    "insertBookConfirmNoButton": "No",
    "remoconCloseButton": "Close",
    "remoconBackButton": "Back",
    "remoconSearchButton": "Search",
    "remoconPlayListButton": "Playlist",
    "remoconHistoryButton": "History",
    "remoconFavoriteButton": "Favorite",
    "remoconLogoutButton": "Logout",
    "downVolRemoconButton": "Vol-",
    "upVolRemoconButton": "Vol+",
    "rewindRemoconButton": "REW",
    "fastForwardRemoconButton": "FF",
    "switchAudioRemoconButton": "Track",
    "pauseRemoconButton": "Pause",
    "stopRemoconButton": "Stop",
    "resumeButton": "Resume",
    "noVideoState": "There is no video playing.",
    "playVideoState": "Playing",
    "pauseVideoState": "Pause",
    "volStartLabel": "Volume: ",
    "volEndLabel": " %",
    "playListCloseButton": "Close",
    "playListBackButton": "Back",
    "playListRemoconButton": "Remote",
    "playListSearchButton": "Search",
    "playListHistoryButton": "History",
    "playListFavoriteButton": "Favorite",
    "playListLogoutButton": "Logout",
    "playListHeaderLabel": "Playlist",
    "restTimeHeadLabel": "Rest time：",
    "finishTimeHeadLabel": "Scheduled end time：",
    "addBreakButton": "Add break",
    "insertBreakButton": "Insert break",
    "addBreakConfirmHeader": "Confirm",
    "addBreakConfirmLabel": "Do you add break on the tail of the playlist?",
    "addBreakConfirmYesButton": "Yes",
    "addBreakConfirmNoButton": "No",
    "insertBreakConfirmHeader": "Confirm",
    "insertBreakConfirmLabel": "Do you insert break into the top of the playlist?",
    "insertBreakConfirmYesButton": "Yes",
    "insertBreakConfirmNoButton": "No",
    "historyListCloseButton": "Close",
    "historyListBackButton": "Back",
    "historyListRemoconButton": "Remote",
    "historyListPlayListButton": "Playlist",
    "historyListSearchButton": "Search",
    "historyListFavoriteButton": "Favorite",
    "historyListLogoutButton": "Logout",
    "historyListHeader": "History",
    "historyListPrevButton": "Prev",
    "historyListNextButton": "Next",
    "historyDetailCloseButton": "Close",
    "historyDetailBackButton": "Back",
    "historyDetailRemoconButton": "Remote",
    "historyDetailPlayListButton": "Playlist",
    "historyDetailSearchButton": "Search",
    "historyDetailFavoriteButton": "Favorite",
    "historyDetailLogoutButton": "Logout",
    "historyDetailHeader": "Detail of History",
    "historyDetailCommentButton": "Modify Comment",
    "historyDetailDeleteButton": "Delete",
    "historyDetailCSVButton": "CSV",
    "modifyHistoryYesButton": "Yes",
    "modifyHistoryNoButton": "No",
    "modifyHistoryHeader": "Confirm",
    "modifyHistoryMessage": "Do you modify the comment?",
    "deleteHistoryHeader": "Confirm",
    "deleteHistoryMessage": "Do you delete the history?",
    "deleteHistoryYesButton": "Yes",
    "deleteHistoryNoButton": "No",
    "downloadHistoryHeader": "Confirm",
    "downloadHistoryMessage": "Do you download a history list that is made by after it? ",
    "downloadHistoryYesButton": "Yes",
    "downloadHistoryNoButton": "No",
    "configAudioButton": "Audio Device",
    "configPlayModeButton": "Play Mode",
    "playModeConfigRequestLabel": "Manual Request Play Mode",
    "playModeConfigShuffleLabel": "Automatic Shuffle Play Mode",
    "playModeConfigTitleLabel": "Play Mode",
    "playModeConfigBackButton": "Back",
    "playModeConfigChangeButton": "Config",
    "audioConfigChangeButton": "Config",
    "audioConfigTitleLabel": "Audio Device",
    "audioConfigBackButton": "Back",
    "audioConfigConfirmHeader": "Confirm",
    "audioConfigConfirmQuestion": "Do you change audio device for use?",
    "audioConfigConfirmAttention": "The change is enabled after the play.",
    "audioConfigConfirmYesButton": "Yes",
    "audioConfigConfirmNoButton": "No",
    "aPConfigTitleLabel": "Access Point",
    "aPConfigBackButton": "Back",
    "aPConfigLabel": "Invalid within 8-characters(Need Reboot)",
    "aPConfigChangeButton": "Config",
    "aPConfigEnableLabel": "Enable",
    "aPConfigDisableLabel": "Disable",
    "stopConfirmHeader": "Confirm",
    "stopConfirmMessage": "Do you stop this video?",
    "stopConfirmYesButton": "Yes",
    "stopConfirmNoButton": "No",
    "breakLabel": "Break",
    "secretLabel": "Secret",
    "bookDetailHeader": "Detail of Reservation",
    "bookDetailCloseButton": "Close",
    "bookDetailBackButton": "Back",
    "bookDetailRemoconButton": "Remote",
    "bookDetailSearchButton": "Search",
    "bookDetailPlayListButton": "Playlist",
    "bookDetailHistoryButton": "History",
    "bookDetailLogoutButton": "Logout",
    "bookDetailUpButton": "Move up",
    "bookDetailRemoveButton": "Remove",
    "bookDetailDownButton": "Move down",
    "deleteBookConfirmHeader": "Confirm",
    "deleteBookConfirmMessage": "Do you remove it from the playlist?",
    "deleteBookConfirmYesButton": "Yes",
    "deleteBookConfirmNoButton": "No"
  }
};
$(function()
{
  $("#searchPage").swiperight(function()
  {
    $("#searchMenu").panel("open");
  });
  $("#fileListPage").swiperight(function()
  {
    $("#fileListMenu").panel("open");
  });
  $("#fileDetailPage").swiperight(function()
  {
    $("#fileDetailMenu").panel("open");
  });
  $("#remoconPage").swiperight(function()
  {
    $("#remoconMenu").panel("open");
  });
  $("#playListPage").swiperight(function()
  {
    $("#playListMenu").panel("open");
  });
  $("#bookDetailPage").swiperight(function()
  {
    $("#bookDetailMenu").panel("open");
  });
  $("#historyListPage").swiperight(function()
  {
    $("#historyListMenu").panel("open");
  });
  $("#historyDetailPage").swiperight(function()
  {
    $("#historyDetailMenu").panel("open");
  });
  setInterval(checkBook, 1000);
});

function checkBook()
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function()
  {
    var json = $.parseJSON(this.responseText);
    if (latestBookFlag)
    {
      var fileName = "";
      if (true == json.visible)
      {
        fileName = json.file.substr(json.file.lastIndexOf("/") + 1) + "を";
      }
      if (null != json.visible && 
          json.bookId != latestBookId && 
          userId != json.userId)
      {
        toast(json.user + "さんが" +
          fileName + "予約しました。" +
          json.comment);
      }
    }
    latestBookFlag = true;
    latestBookId = json.bookId;
  });
  xhr.open("POST", window.origin);
  xhr.send("B");
}

$(document).on('pagehide', '#fileDetailPage', function(){
  $("#previewVideo").attr("src","");
}
);
$(document).on('pagebeforeshow', '#fileDetailPage', function(){
  $("#previewVideo").attr("src",fileId);
}
);
$(document).on('pagecreate', '#loginPage', function() 
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function(){
    var json = $.parseJSON(this.responseText);
    ssid = json.ssid;
    pass = json.pass;
    $("#ssidLabel").text("SSID:" + ssid);
    $("#wifiPassLabel").text("PASS:" + pass);
    $("#urlLabel").text(window.origin);
  });
  xhr.open("POST", window.origin);
  var day = new Date();
  xhr.send("1" + 
    day.getFullYear() + "/" +
    String(Number(Number(day.getMonth())+1)) + "/" +
    day.getDate() + " " +
    day.getHours() + ":" +
    day.getMinutes() + ":" +
    day.getSeconds());
}
);
function playListInterval()
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function(){
    var json = $.parseJSON(this.responseText).playList;
    var li = "";
    var rest = $.parseJSON(this.responseText).restCurrentTime;
    for(var i=0;i<json.length;++i)
    {
      rest += json[i].duration;
      if (0 <= json[i].path.indexOf("/"))
      {
        li += '<li style="white-space:pre-line;">';
        li += '<a href="javascript:void(0)" onclick="detailBook(' + json[i].bookId + ',\'' + json[i].user + '\',\'' + json[i].path + '\',' + json[i].duration +',\'' + json[i].comment + '\');">';
        if ("" == json[i].user)
        {
          li += "<br>";
        }
        else
        {
          li += json[i].user + "<br>";
        }
        li += json[i].path.substr(json[i].path.lastIndexOf("/") + 1) + "<br>";
        if ("" != json[i].comment)
        {
          li += json[i].comment + "<br>";
        }
        li += '</a>';
        li += '</li>';
      }
      else if (0 == json[i].path.indexOf("Break"))
      {
        li += '<li style="white-space:pre-line;">';
        li += '<a href="javascript:void(0)" onclick="detailBook(' + json[i].bookId + ',\'' + locale[language]["breakLabel"] + '\',\'\',0,\'\');">';
        li += locale[language]["breakLabel"];
        li += '</a>';
        li += '</li>';
      }
      else
      {
        li += '<li style="white-space:pre-line;">';
        li += '<a href="javascript:void(0)" onclick="detailBook(' + json[i].bookId + ',\'' + json[i].user + '\',\'' + locale[language]["secretLabel"] + '\',' + json[i].duration +',\'' + json[i].comment + '\');">';
        if ("" == json[i].user)
        {
          li += "<br>";
        }
        else
        {
          li += json[i].user + "<br>";
        }
        li += locale[language]["secretLabel"] + "<br>";
        if ("" != json[i].comment)
        {
          li += json[i].comment + "<br>";
        }
        li += '</a>';
        li += '</li>';
      }
    }
    $("#playListview").empty();
    $("#playListview").append(li);
    $("#playListview").listview();
    $("#playListview").listview("refresh");

    var hour = ('00' + String(Math.floor(rest / 3600))).slice(-2);
    var min = ('00' + String(Math.floor((rest % 3600) / 60))).slice(-2);
    var sec = ('00' + String(rest % 60)).slice(-2);

    $("#restTimeLabel").text(locale[language]["restTimeHeadLabel"] + hour + ":" + min + ":" + sec);
    var dt = new Date();
    dt.setSeconds(dt.getSeconds() + rest);
    $("#finishTimeLabel").text(locale[language]["finishTimeHeadLabel"] + dt.toLocaleTimeString());
  });
  xhr.open("POST", window.origin);
  xhr.send("4");
}
$(document).on('pageshow', '#playListPage', function()
{
  playListTimer = setInterval(playListInterval, 250);
}
);
$(document).on('pagehide', '#playListPage', function()
{
  clearTimeout(playListTimer);
}
);
function remoconInterval()
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function(){
    var json = $.parseJSON(this.responseText);
    if (0 == json.state || 3 == json.state)
    {
      $("#singerNameRemoconLabel").hide();
      $("#commentRemoconLabel").hide();
      $("#pathRemoconLabel").hide();
      $("#durationRemoconLabel").hide();
      $("#playerStateLabel").text(locale[language]["noVideoState"]);
      $("#playSongProgress").hide();
      $("#volRemoconLabel").text(locale[language]["volStartLabel"] + json.vol + locale[language]["volEndLabel"]);
      $("#rewindRemoconButton").hide();
      $("#fastForwardRemoconButton").hide();
      $("#switchAudioRemoconButton").hide();
      $("#stopRemoconButton").hide();
      if (0 == json.state)
      {
        $("#pauseRemoconButton").text(locale[language]["pauseRemoconButton"]);
      }
      else if (3 == json.state)
      {
        $("#pauseRemoconButton").text(locale[language]["resumeButton"]);
      }
    }
    else if (1 == json.state || 2 == json.state)
    {
      $("#singerNameRemoconLabel").text(json.user);
      $("#singerNameRemoconLabel").show();
      $("#commentRemoconLabel").text(json.comment);
      $("#commentRemoconLabel").show();
      $("#pathRemoconLabel").text(json.path);
      $("#pathRemoconLabel").show();
      var posHour = ('00' + String(Math.floor(json.pos / 3600))).slice(-2);
      var posMin = ('00' + String(Math.floor((json.pos % 3600) / 60))).slice(-2);
      var posSec = ('00' + String(json.pos % 60)).slice(-2);
      var durationHour = ('00' + String(Math.floor(json.duration / 3600))).slice(-2);
      var durationMin = ('00' + String(Math.floor((json.duration % 3600) / 60))).slice(-2);
      var durationSec = ('00' + String(json.duration % 60)).slice(-2);
      $("#durationRemoconLabel").text(posHour + ":" + posMin + ":" + posSec + "/" + durationHour + ":" + durationMin + ":" + durationSec);
      $("#durationRemoconLabel").show();
      if (1 == json.state)
      {
        $("#playerStateLabel").text(locale[language]["playVideoState"]);
      }
      else if (2 == json.state)
      {
        $("#playerStateLabel").text(locale[language]["pauseVideoState"]);
      }
      $("#playSongProgress").prop("value", json.pos);
      $("#playSongProgress").prop("max", json.duration);
      $("#playSongProgress").show();
      $("#volRemoconLabel").text(locale[language]["volStartLabel"] + json.vol + locale[language]["volEndLabel"]);
      $("#rewindRemoconButton").show();
      $("#fastForwardRemoconButton").show();
      $("#switchAudioRemoconButton").show();
      $("#stopRemoconButton").show();
      if (1 == json.state)
      {
        $("#pauseRemoconButton").text(locale[language]["pauseRemoconButton"]);
      }
      else if (2 == json.state)
      {
        $("#pauseRemoconButton").text(locale[language]["resumeButton"]);
      }
    }
  });
  xhr.open("POST", window.origin);
  xhr.send("X");
}

$(document).on('pageshow', '#remoconPage', function()
{
  remoconTimer = setInterval(remoconInterval, 250);
}
);
$(document).on('pagehide', '#remoconPage', function()
{
  clearTimeout(remoconTimer);
}
);
function applyLanguage(list)
{
  for (i=0; i<list.length; ++i)
  {
    if (list[i].id === "")
    {
      continue;
    }
    try
    {
      $("#"+list[i].id).text(locale[language][list[i].id]);
    }
    catch(e)
    {
    }
  }
}
function setLanguage(lang)
{
  language = lang;
  list = document.getElementsByTagName("a");
  applyLanguage(list);
  list = document.getElementsByTagName("span");
  applyLanguage(list);
  list = document.getElementsByTagName("option");
  applyLanguage(list);
  list = document.getElementsByTagName("label");
  applyLanguage(list);
  list = document.getElementsByTagName("input");
  applyLanguage(list);
  window.location.href="#loginPage";
}
function login()
{
  if ($("#userNameTextbox").val() == "")
  {
    alert(locale[language]["loginAlert"]);
    userId = 0;
    $("#userNameSearchLabel").text("");
    window.location.href="#searchPage";
  }
  else
  {
    $.mobile.loading('show');
    var xhr = new XMLHttpRequest();
    xhr.timeout = 1000;
    xhr.addEventListener("load", function(){
      userId = this.responseText;
      $.mobile.loading('hide');
      window.location.href="#searchPage";
    });
    xhr.addEventListener("timeout", function(){
      alert(locale[language]["unableConnectAlert"]);
      $.mobile.loading('hide');
    });
    xhr.open("POST", window.origin);
    xhr.send("0" + $("#userNameTextbox").val());
    $("#userNameSearchLabel").text($("#userNameTextbox").val());
    userId = null;
  }
}
function listFile(page)
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function(){
    $("#fileListKeywordLabel").text($("#searchTextbox").val());
    if (page - 2 > 0)
    {
      $("#fileListLeftTwoButton").text(page - 2);
      $("#fileListLeftTwoButton").show();
    }
    else
    {
      $("#fileListLeftTwoButton").hide();
    }
    if (page - 1 > 0)
    {
      $("#fileListLeftOneButton").text(page - 1);
      $("#fileListLeftOneButton").show();
    }
    else
    {
      $("#fileListLeftOneButton").hide();
    }
    $("#fileListPageIndexLabel").text(page);
    $("#fileListPageIndexLabel").show();
    $("#fileListRightOneButton").text(page + 1);
    $("#fileListRightTwoButton").text(page + 2);
    $("#fileListPrevButton").css("display", "inline-block");
    $("#fileListNextButton").css("display", "inline-block");
    $("#fileListHeadButton").css("display", "inline-block");
    $("#fileListHeadNextButton").css("display", "inline-block");
    $("#fileListLeftSkipLabel").css("display", "inline-block");
    $("#fileListLeftTwoButton").css("display", "inline-block");
    $("#fileListLeftOneButton").css("display", "inline-block");
    $("#fileListLeftOneButton").css("display", "inline-block");
    $("#fileListRightOneButton").css("display", "inline-block");
    $("#fileListRightTwoButton").css("display", "inline-block");
    $("#fileListRightSkipLabel").css("display", "inline-block");
    $("#fileListBottomPrevButton").css("display", "inline-block");
    $("#fileListBottomButton").css("display", "inline-block");
    $("#fileListCRLabel").hide();
    $("#fileListLFLabel").hide();
    var json = $.parseJSON(this.responseText);
    $("#fileListBottomButton").text(json.page);
    $("#fileListBottomPrevButton").text(json.page - 1);
    $("#fileListRecordNumLabel").text(json.record + locale[language]["result"]);
    if (0 == json.record)
    {
      $("#fileListNextButton").hide();
      $("#fileListPageIndexLabel").hide();
      $("#fileListRightOneButton").hide();
      $("#fileListLeftOneButton").hide();
      $("#fileListRightTwoButton").hide();
      $("#fileListLeftTwoButton").hide();
      $("#fileListRightSkipLabel").hide();
      $("#fileListLeftSkipLabel").hide();
      $("#fileListBottomPrevButton").hide();
      $("#fileListBottomButton").hide();
      $("#fileListPrevButton").hide();
      $("#fileListHeadButton").hide();
      $("#fileListHeadNextButton").hide();
    }
    else if (1 == json.page)
    {
      $("#fileListPrevButton").hide();
      $("#fileListNextButton").hide();
      $("#fileListHeadButton").hide();
      $("#fileListHeadNextButton").hide();
      $("#fileListLeftSkipLabel").hide();
      $("#fileListLeftTwoButton").hide();
      $("#fileListLeftOneButton").hide();
      $("#fileListRightOneButton").hide();
      $("#fileListRightTwoButton").hide();
      $("#fileListRightSkipLabel").hide();
      $("#fileListBottomPrevButton").hide();
      $("#fileListBottomButton").hide();
    }
    else
    {
      if (1 == page)
      {
        $("#fileListPrevButton").hide();
        $("#fileListNextButton").off('click');
        $("#fileListNextButton").on('click', function(){listFile(page + 1);});
        $("#fileListHeadButton").hide();
        $("#fileListHeadNextButton").hide();
        $("#fileListLeftSkipLabel").hide();
        $("#fileListLeftTwoButton").hide();
        $("#fileListLeftOneButton").hide();
      }
      else if (2 == page)
      {
        $("#fileListPrevButton").off('click');
        $("#fileListPrevButton").on('click', function(){listFile(page - 1);});
        $("#fileListNextButton").off('click');
        $("#fileListNextButton").on('click', function(){listFile(page + 1);});
        $("#fileListHeadButton").hide();
        $("#fileListHeadNextButton").hide();
        $("#fileListLeftSkipLabel").hide();
        $("#fileListLeftTwoButton").hide();
        $("#fileListLeftOneButton").text("1");
        $("#fileListLeftOneButton").off('click');
        $("#fileListLeftOneButton").on('click', function(){listFile(1);});
      }
      else if (3 == page)
      {
        $("#fileListPrevButton").off('click');
        $("#fileListPrevButton").on('click', function(){listFile(page - 1);});
        $("#fileListNextButton").off('click');
        $("#fileListNextButton").on('click', function(){listFile(page + 1);});
        $("#fileListHeadButton").hide();
        $("#fileListHeadNextButton").hide();
        $("#fileListLeftSkipLabel").hide();
        $("#fileListLeftTwoButton").text("1");
        $("#fileListLeftTwoButton").off('click');
        $("#fileListLeftTwoButton").on('click', function(){listFile(1);});
        $("#fileListLeftOneButton").text("2");
        $("#fileListLeftOneButton").off('click');
        $("#fileListLeftOneButton").on('click', function(){listFile(2);});
      }
      else if (4 == page)
      {
        $("#fileListPrevButton").off('click');
        $("#fileListPrevButton").on('click', function(){listFile(page - 1);});
        $("#fileListNextButton").off('click');
        $("#fileListNextButton").on('click', function(){listFile(page + 1);});
        $("#fileListHeadButton").hide();
        $("#fileListHeadNextButton").text("1");
        $("#fileListHeadNextButton").off('click');
        $("#fileListHeadNextButton").on('click', function(){listFile(1);});
        $("#fileListLeftSkipLabel").hide();
        $("#fileListLeftTwoButton").text("2");
        $("#fileListLeftTwoButton").off('click');
        $("#fileListLeftTwoButton").on('click', function(){listFile(2);});
        $("#fileListLeftOneButton").text("3");
        $("#fileListLeftOneButton").off('click');
        $("#fileListLeftOneButton").on('click', function(){listFile(3);});
      }
      else if (5 == page)
      {
        $("#fileListPrevButton").off('click');
        $("#fileListPrevButton").on('click', function(){listFile(page - 1);});
        $("#fileListNextButton").off('click');
        $("#fileListNextButton").on('click', function(){listFile(page + 1);});
        $("#fileListHeadButton").text("1");
        $("#fileListHeadButton").off('click');
        $("#fileListHeadButton").on('click', function(){listFile(1);});
        $("#fileListHeadNextButton").text("2");
        $("#fileListHeadNextButton").off('click');
        $("#fileListHeadNextButton").on('click', function(){listFile(2);});
        $("#fileListLeftSkipLabel").hide();
        $("#fileListLeftTwoButton").text("3");
        $("#fileListLeftTwoButton").off('click');
        $("#fileListLeftTwoButton").on('click', function(){listFile(3);});
        $("#fileListLeftOneButton").text("4");
        $("#fileListLeftOneButton").off('click');
        $("#fileListLeftOneButton").on('click', function(){listFile(4);});
      }
      else if (5 < page)
      {
        $("#fileListPrevButton").off('click');
        $("#fileListPrevButton").on('click', function(){listFile(page - 1);});
        $("#fileListNextButton").off('click');
        $("#fileListNextButton").on('click', function(){listFile(page + 1);});
        $("#fileListHeadButton").text("1");
        $("#fileListHeadButton").off('click');
        $("#fileListHeadButton").on('click', function(){listFile(1);});
        $("#fileListHeadNextButton").text("2");
        $("#fileListHeadNextButton").off('click');
        $("#fileListHeadNextButton").on('click', function(){listFile(2);});
        $("#fileListLeftTwoButton").text(page - 2);
        $("#fileListLeftTwoButton").off('click');
        $("#fileListLeftTwoButton").on('click', function(){listFile(page - 2);});
        $("#fileListLeftOneButton").text(page - 1);
        $("#fileListLeftOneButton").off('click');
        $("#fileListLeftOneButton").on('click', function(){listFile(page - 1);});
      }
      if (page == json.page)
      {
        //$("#fileListCRLabel").show();
        $("#fileListNextButton").hide();
        $("#fileListRightOneButton").hide();
        $("#fileListRightTwoButton").hide();
        $("#fileListRightSkipLabel").hide();
        $("#fileListBottomPrevButton").hide();
        $("#fileListBottomButton").hide();
        //$("#fileListLFLabel").show();
      }
      else if ((page + 1) == json.page)
      {
        $("#fileListRightOneButton").text(page + 1);
        $("#fileListRightOneButton").off('click');
        $("#fileListRightOneButton").on('click', function(){listFile(page + 1);});
        $("#fileListRightTwoButton").hide();
        $("#fileListRightSkipLabel").hide();
        $("#fileListBottomPrevButton").hide();
        $("#fileListBottomButton").hide();
      }
      else if ((page + 2) == json.page)
      {
        $("#fileListRightOneButton").text(page + 1);
        $("#fileListRightOneButton").off('click');
        $("#fileListRightOneButton").on('click', function(){listFile(page + 1);});
        $("#fileListRightTwoButton").text(page + 2);
        $("#fileListRightTwoButton").off('click');
        $("#fileListRightTwoButton").on('click', function(){listFile(page + 2);});
        $("#fileListRightSkipLabel").hide();
        $("#fileListBottomPrevButton").hide();
        $("#fileListBottomButton").hide();
      }
      else if ((page + 3) == json.page)
      {
        $("#fileListRightOneButton").text(page + 1);
        $("#fileListRightOneButton").off('click');
        $("#fileListRightOneButton").on('click', function(){listFile(page + 1);});
        $("#fileListRightTwoButton").text(page + 2);
        $("#fileListRightTwoButton").off('click');
        $("#fileListRightTwoButton").on('click', function(){listFile(page + 2);});
        $("#fileListRightSkipLabel").hide();
        $("#fileListBottomPrevButton").text(page + 3);
        $("#fileListBottomPrevButton").off('click');
        $("#fileListBottomPrevButton").on('click', function(){listFile(page + 3);});
        $("#fileListBottomButton").hide();
      }
      else if ((page + 4) == json.page)
      {
        $("#fileListRightOneButton").text(page + 1);
        $("#fileListRightOneButton").off('click');
        $("#fileListRightOneButton").on('click', function(){listFile(page + 1);});
        $("#fileListRightTwoButton").text(page + 2);
        $("#fileListRightTwoButton").off('click');
        $("#fileListRightTwoButton").on('click', function(){listFile(page + 2);});
        $("#fileListRightSkipLabel").hide();
        $("#fileListBottomPrevButton").text(page + 3);
        $("#fileListBottomPrevButton").off('click');
        $("#fileListBottomPrevButton").on('click', function(){listFile(page + 3);});
        $("#fileListBottomButton").text(page + 4);
        $("#fileListBottomButton").off('click');
        $("#fileListBottomButton").on('click', function(){listFile(page + 4);});
      }
      else if ((page + 4) < json.page)
      {
        $("#fileListRightOneButton").text(page + 1);
        $("#fileListRightOneButton").off('click');
        $("#fileListRightOneButton").on('click', function(){listFile(page + 1);});
        $("#fileListRightTwoButton").text(page + 2);
        $("#fileListRightTwoButton").off('click');
        $("#fileListRightTwoButton").on('click', function(){listFile(page + 2);});
        $("#fileListBottomPrevButton").text(json.page - 1);
        $("#fileListBottomPrevButton").off('click');
        $("#fileListBottomPrevButton").on('click', function(){listFile(json.page - 1);});
        $("#fileListBottomButton").text(json.page);
        $("#fileListBottomButton").off('click');
        $("#fileListBottomButton").on('click', function(){listFile(json.page);});
      }
    }
    $("#fileListView").empty();
    for(var i=0; i<json.files.length; ++i)
    {
      if (false == json.files[i].played)
      {
        $("#fileListView").append(
          '<li>' +
          '<a href="javascript:void(0)" onclick="fileDetail(' + 
          json.files[i].fileId + ');">' +
          '<img src="t/' + json.files[i].fileId + '.png">' +
          '<font style="white-space:normal;">' +
          json.files[i].name +
          '</font>' +
          '</a>' +
          '</li>'
        );
      }
      else
      {
        $("#fileListView").append(
          '<li>' +
          '<a href="javascript:void(0)" onclick="fileDetail(' + 
          json.files[i].fileId + 
          ');" class="ui-btn ui-icon-check ui-btn-icon-left">' +
          '<img src="t/' + json.files[i].fileId + '.png">' +
          json.files[i].name +
          '</a>' +
          '</li>'
        );
      }
    }
    $("#fileListView").listview();
    $("#fileListView").listview('refresh');
    window.location.href="#fileListPage";
  });
  xhr.open("POST", window.origin);
  xhr.send("2" + $("#sortSelect").val() + page + "," + $("#searchTextbox").val());
}
function fileDetail(fileId)
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function(){
    $("#userNameFileDetailLabel").text($("#userNameTextbox").val());
    var responseText = this.responseText.replace(/\u000c/g,"");
    var json = $.parseJSON(responseText);
    $("#pathFileDetailLabel").text(json.path);
    //$("#sizeFileDetailLabel").text(json.size.toLocaleString() + "[Byte]");
    $("#timeFileDetailLabel").text(json.date);
    var hour = ('00' + String(Math.floor(json.duration / 3600))).slice(-2);
    var min = ('00' + String(Math.floor((json.duration % 3600) / 60))).slice(-2);
    var sec = ('00' + String(json.duration % 60)).slice(-2);
    $("#durationFileDetailLabel").text(hour + ":" + min + ":" + sec);
    $("#videoFileDetailLabel").text(json.videoCodec + "(" + json.resolution + ")");
    $("#countFileDetailLabel").text(json.count + locale[language]["playCountUnit"]);
    var optItem = '';
    for (var i=0;i<json.audioTrack.length;++i)
    {
      optItem += '<option value="' + i + '">' + json.audioTrack[i] + '</option>';
    }
    $("#audioAddBookSelect").empty();
    $("#audioAddBookSelect").append(optItem);
    $("#audioAddBookSelect").val(0);

    $("#audioInsertBookSelect").empty();
    $("#audioInsertBookSelect").append(optItem);
    $("#audioInsertBookSelect").val(0);

    $("#audioSwapBookSelect").empty();
    $("#audioSwapBookSelect").append(optItem);
    $("#audioSwapBookSelect").val(0);

    $("#audioAddBookSelect").selectmenu();
    $("#audioAddBookSelect").selectmenu('refresh', true);
    $("#audioInsertBookSelect").selectmenu();
    $("#audioInsertBookSelect").selectmenu('refresh', true);
    $("#audioSwapBookSelect").selectmenu();
    $("#audioSwapBookSelect").selectmenu('refresh', true);
    $("#previewVideo").attr("src",fileId);
    window.location.href="#fileDetailPage";
  });
  xhr.open("POST", window.origin);
  xhr.send("3" + fileId);
  window.fileId = fileId;
}
function remocon()
{
  $("#userNameRemoconLabel").text($("#userNameTextbox").val());
  remoconInterval();
  window.location.href="#remoconPage";
}
function playList()
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function(){
    var json = $.parseJSON(this.responseText).playList;
    var li = "";
    var rest = 0;
    for(var i=0;i<json.length;++i)
    {
      rest += json[i].duration;
      if (0 <= json[i].path.indexOf("/"))
      {
        li += '<li style="white-space:pre-line;">';
        li += '<a href="javascript:void(0)" onclick="detailBook(' + json[i].bookId + ',\'' + json[i].user + '\',\'' + json[i].path + '\',' + json[i].duration +',\'' + json[i].comment + '\');">';
        if ("" == json[i].user)
        {
          li += "<br>";
        }
        else
        {
          li += json[i].user + "<br>";
        }
        li += json[i].path.substr(json[i].path.lastIndexOf("/") + 1) + "<br>";
        if ("" != json[i].comment)
        {
          li += json[i].comment + "<br>";
        }
        li += '</a>';
        li += '</li>';
      }
      else if (0 == json[i].path.indexOf("Break"))
      {
        li += '<li style="white-space:pre-line;">';
        li += '<a href="javascript:void(0)" onclick="detailBook(' + json[i].bookId + ',\'' + locale[language]["breakLabel"] + '\',\'\',0,\'\');">';
        li += locale[language]["breakLabel"];
        li += '</a>';
        li += '</li>';
      }
      else
      {
        li += '<li style="white-space:pre-line;">';
        li += '<a href="javascript:void(0)" onclick="detailBook(' + json[i].bookId + ',\'' + json[i].user + '\',\'' + locale[language]["secretLabel"] + '\',' + json[i].duration +',\'' + json[i].comment + '\');">';
        if ("" == json[i].user)
        {
          li += "<br>";
        }
        else
        {
          li += json[i].user + "<br>";
        }
        li += locale[language]["secretLabel"] + "<br>";
        if ("" != json[i].comment)
        {
          li += json[i].comment + "<br>";
        }
        li += '</a>';
        li += '</li>';
      }
    }
    $("#playListview").empty();
    $("#playListview").append(li);
    $("#playListview").listview();
    $("#playListview").listview("refresh");

    var hour = ('00' + String(Math.floor(rest / 3600))).slice(-2);
    var min = ('00' + String(Math.floor((rest % 3600) / 60))).slice(-2);
    var sec = ('00' + String(rest % 60)).slice(-2);

    $("#restTimeLabel").text(locale[language]["restTimeHeadLabel"] + hour + ":" + min + ":" + sec);
    var dt = new Date();
    dt.setSeconds(dt.getSeconds() + rest);
    $("#finishTimeLabel").text(locale[language]["finishTimeHeadLabel"] + dt.toLocaleTimeString());
    if ("#playListPage" != window.location.href)
    {
      window.location.href="#playListPage";
    }
  });
  xhr.open("POST", window.origin);
  xhr.send("4");
}
function historyList(page)
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function(){
    $("#userNameHistoryListLabel").text($("#userNameTextbox").val());
    var json = $.parseJSON(this.responseText);
    $("#historyListRecord").text(json.record + locale[language]["result"]);
    historyListJson = json;
    var li = "";
    for (var i=0; i<json.histories.length; ++i)
    {
      li += '<li style="white-space:pre-line;">';
      li += '<a href="javascript:void(0)" ';
      li += 'onclick=\'historyDetail(' + i + ');\'>';
      li += json.histories[i].user + '<br>';
      li += json.histories[i].path.substr(json.histories[i].path.lastIndexOf("/")+1) + "<br>";
      li += json.histories[i].comment;
      li += '</a></li>';
    }
    $("#historyListview").empty();
    $("#historyListview").append(li);
    $("#historyListview").listview();
    $("#historyListview").listview("refresh");
    if (page - 2 > 0)
    {
      $("#historyListLeftTwoButton").text(page - 2);
    }
    else
    {
      $("#historyListLeftTwoButton").hide();
    }
    if (page - 1 > 0)
    {
      $("#historyListLeftOneButton").text(page - 1);
    }
    else
    {
      $("#historyListLeftOneButton").hide();
    }
    $("#historyListPageIndexLabel").text(page);
    $("#historyListRightOneButton").text(page + 1);
    $("#historyListRightTwoButton").text(page + 2);
    $("#historyListPrevButton").css("display", "inline-block");
    $("#historyListNextButton").css("display", "inline-block");
    $("#historyListHeadButton").css("display", "inline-block");
    $("#historyListHeadNextButton").css("display", "inline-block");
    $("#historyListLeftSkipLabel").css("display", "inline-block");
    $("#historyListLeftTwoButton").css("display", "inline-block");
    $("#historyListLeftOneButton").css("display", "inline-block");
    $("#historyListLeftOneButton").css("display", "inline-block");
    $("#historyListRightOneButton").css("display", "inline-block");
    $("#historyListRightTwoButton").css("display", "inline-block");
    $("#historyListRightSkipLabel").css("display", "inline-block");
    $("#historyListBottomPrevButton").css("display", "inline-block");
    $("#historyListBottomButton").css("display", "inline-block");
    $("#historyListCRLabel").hide();
    $("#historyListLFLabel").hide();
    $("#historyListBottomButton").text(json.page);
    $("#historyListBottomPrevButton").text(json.page - 1);
    if (0 == json.record)
    {
      $("#historyListNextButton").hide();
      $("#historyListPageIndexLabel").hide();
      $("#historyListRightOneButton").hide();
      $("#historyListLeftOneButton").hide();
      $("#historyListRightTwoButton").hide();
      $("#historyListLeftTwoButton").hide();
      $("#historyListRightSkipLabel").hide();
      $("#historyListLeftSkipLabel").hide();
      $("#historyListBottomPrevButton").hide();
      $("#historyListBottomButton").hide();
      $("#historyListPrevButton").hide();
      $("#historyListHeadButton").hide();
      $("#historyListHeadNextButton").hide();
    }
    else if (1 == json.page)
    {
      $("#historyListPrevButton").hide();
      $("#historyListNextButton").hide();
      $("#historyListHeadButton").hide();
      $("#historyListHeadNextButton").hide();
      $("#historyListLeftSkipLabel").hide();
      $("#historyListLeftTwoButton").hide();
      $("#historyListLeftOneButton").hide();
      $("#historyListRightOneButton").hide();
      $("#historyListRightTwoButton").hide();
      $("#historyListRightSkipLabel").hide();
      $("#historyListBottomPrevButton").hide();
      $("#historyListBottomButton").hide();
    }
    else
    {
      if (1 == page)
      {
        $("#historyListPrevButton").hide();
        $("#historyListNextButton").off('click');
        $("#historyListNextButton").on('click', function(){historyList(page + 1);});
        $("#historyListHeadButton").hide();
        $("#historyListHeadNextButton").hide();
        $("#historyListLeftSkipLabel").hide();
        $("#historyListLeftTwoButton").hide();
        $("#historyListLeftOneButton").hide();
      }
      else if (2 == page)
      {
        $("#historyListPrevButton").off('click');
        $("#historyListPrevButton").on('click', function(){historyList(page - 1);});
        $("#historyListNextButton").off('click');
        $("#historyListNextButton").on('click', function(){historyList(page + 1);});
        $("#historyListHeadButton").hide();
        $("#historyListHeadNextButton").hide();
        $("#historyListLeftSkipLabel").hide();
        $("#historyListLeftTwoButton").hide();
        $("#historyListLeftOneButton").text("1");
        $("#historyListLeftOneButton").off('click');
        $("#historyListLeftOneButton").on('click', function(){historyList(1);});
      }
      else if (3 == page)
      {
        $("#historyListPrevButton").off('click');
        $("#historyListPrevButton").on('click', function(){historyList(page - 1);});
        $("#historyListNextButton").off('click');
        $("#historyListNextButton").on('click', function(){historyList(page + 1);});
        $("#historyListHeadButton").hide();
        $("#historyListHeadNextButton").hide();
        $("#historyListLeftSkipLabel").hide();
        $("#historyListLeftTwoButton").text("1");
        $("#historyListLeftTwoButton").off('click');
        $("#historyListLeftTwoButton").on('click', function(){historyList(1);});
        $("#historyListLeftOneButton").text("2");
        $("#historyListLeftOneButton").off('click');
        $("#historyListLeftOneButton").on('click', function(){historyList(2);});
      }
      else if (4 == page)
      {
        $("#historyListPrevButton").off('click');
        $("#historyListPrevButton").on('click', function(){historyList(page - 1);});
        $("#historyListNextButton").off('click');
        $("#historyListNextButton").on('click', function(){historyList(page + 1);});
        $("#historyListHeadButton").hide();
        $("#historyListHeadNextButton").text("1");
        $("#historyListHeadNextButton").off('click');
        $("#historyListHeadNextButton").on('click', function(){historyList(1);});
        $("#historyListLeftSkipLabel").hide();
        $("#historyListLeftTwoButton").text("2");
        $("#historyListLeftTwoButton").off('click');
        $("#historyListLeftTwoButton").on('click', function(){historyList(2);});
        $("#historyListLeftOneButton").text("3");
        $("#historyListLeftOneButton").off('click');
        $("#historyListLeftOneButton").on('click', function(){historyList(3);});
      }
      else if (5 == page)
      {
        $("#historyListPrevButton").off('click');
        $("#historyListPrevButton").on('click', function(){historyList(page - 1);});
        $("#historyListNextButton").off('click');
        $("#historyListNextButton").on('click', function(){historyList(page + 1);});
        $("#historyListHeadButton").text("1");
        $("#historyListHeadButton").off('click');
        $("#historyListHeadButton").on('click', function(){historyList(1);});
        $("#historyListHeadNextButton").text("2");
        $("#historyListHeadNextButton").off('click');
        $("#historyListHeadNextButton").on('click', function(){historyList(2);});
        $("#historyListLeftSkipLabel").hide();
        $("#historyListLeftTwoButton").text("3");
        $("#historyListLeftTwoButton").off('click');
        $("#historyListLeftTwoButton").on('click', function(){historyList(3);});
        $("#historyListLeftOneButton").text("4");
        $("#historyListLeftOneButton").off('click');
        $("#historyListLeftOneButton").on('click', function(){historyList(4);});
      }
      else if (5 < page)
      {
        $("#historyListPrevButton").off('click');
        $("#historyListPrevButton").on('click', function(){historyList(page - 1);});
        $("#historyListNextButton").off('click');
        $("#historyListNextButton").on('click', function(){historyList(page + 1);});
        $("#historyListHeadButton").text("1");
        $("#historyListHeadButton").off('click');
        $("#historyListHeadButton").on('click', function(){historyList(1);});
        $("#historyListHeadNextButton").text("2");
        $("#historyListHeadNextButton").off('click');
        $("#historyListHeadNextButton").on('click', function(){historyList(2);});
        $("#historyListLeftTwoButton").text(page - 2);
        $("#historyListLeftTwoButton").off('click');
        $("#historyListLeftTwoButton").on('click', function(){historyList(page - 2);});
        $("#historyListLeftOneButton").text(page - 1);
        $("#historyListLeftOneButton").off('click');
        $("#historyListLeftOneButton").on('click', function(){historyList(page - 1);});
      }
      if (page == json.page)
      {
        $("#historyListCRLabel").show();
        $("#historyListNextButton").hide();
        $("#historyListRightOneButton").hide();
        $("#historyListRightTwoButton").hide();
        $("#historyListRightSkipLabel").hide();
        $("#historyListBottomPrevButton").hide();
        $("#historyListBottomButton").hide();
        $("#historyListLFLabel").show();
      }
      else if ((page + 1) == json.page)
      {
        $("#historyListRightOneButton").text(page + 1);
        $("#historyListRightOneButton").off('click');
        $("#historyListRightOneButton").on('click', function(){historyList(page + 1);});
        $("#historyListRightTwoButton").hide();
        $("#historyListRightSkipLabel").hide();
        $("#historyListBottomPrevButton").hide();
        $("#historyListBottomButton").hide();
      }
      else if ((page + 2) == json.page)
      {
        $("#historyListRightOneButton").text(page + 1);
        $("#historyListRightOneButton").off('click');
        $("#historyListRightOneButton").on('click', function(){historyList(page + 1);});
        $("#historyListRightTwoButton").text(page + 2);
        $("#historyListRightTwoButton").off('click');
        $("#historyListRightTwoButton").on('click', function(){historyList(page + 2);});
        $("#historyListRightSkipLabel").hide();
        $("#historyListBottomPrevButton").hide();
        $("#historyListBottomButton").hide();
      }
      else if ((page + 3) == json.page)
      {
        $("#historyListRightOneButton").text(page + 1);
        $("#historyListRightOneButton").off('click');
        $("#historyListRightOneButton").on('click', function(){historyList(page + 1);});
        $("#historyListRightTwoButton").text(page + 2);
        $("#historyListRightTwoButton").off('click');
        $("#historyListRightTwoButton").on('click', function(){historyList(page + 2);});
        $("#historyListRightSkipLabel").hide();
        $("#historyListBottomPrevButton").text(page + 3);
        $("#historyListBottomPrevButton").off('click');
        $("#historyListBottomPrevButton").on('click', function(){historyList(page + 3);});
        $("#historyListBottomButton").hide();
      }
      else if ((page + 4) == json.page)
      {
        $("#historyListRightOneButton").text(page + 1);
        $("#historyListRightOneButton").off('click');
        $("#historyListRightOneButton").on('click', function(){historyList(page + 1);});
        $("#historyListRightTwoButton").text(page + 2);
        $("#historyListRightTwoButton").off('click');
        $("#historyListRightTwoButton").on('click', function(){historyList(page + 2);});
        $("#historyListRightSkipLabel").hide();
        $("#historyListBottomPrevButton").text(page + 3);
        $("#historyListBottomPrevButton").off('click');
        $("#historyListBottomPrevButton").on('click', function(){historyList(page + 3);});
        $("#historyListBottomButton").text(page + 4);
        $("#historyListBottomButton").off('click');
        $("#historyListBottomButton").on('click', function(){historyList(page + 4);});
      }
      else if ((page + 4) < json.page)
      {
        $("#historyListRightOneButton").text(page + 1);
        $("#historyListRightOneButton").off('click');
        $("#historyListRightOneButton").on('click', function(){historyList(page + 1);});
        $("#historyListRightTwoButton").text(page + 2);
        $("#historyListRightTwoButton").off('click');
        $("#historyListRightTwoButton").on('click', function(){historyList(page + 2);});
        $("#historyListBottomPrevButton").text(json.page - 1);
        $("#historyListBottomPrevButton").off('click');
        $("#historyListBottomPrevButton").on('click', function(){historyList(json.page - 1);});
        $("#historyListBottomButton").text(json.page);
        $("#historyListBottomButton").off('click');
        $("#historyListBottomButton").on('click', function(){historyList(json.page);});
      }
    }
    window.location.href="#historyListPage";
  });
  xhr.open("POST", window.origin);
  xhr.send("C" + page);
}

function historyDetail(historyIndex)
{
  historyId = historyListJson.histories[historyIndex].historyId;
  $("#playTimeHistoryLabel").text(historyListJson.histories[historyIndex].time);
  $("#pathHistoryLabel").text(historyListJson.histories[historyIndex].path);
  $("#userNameHistoryLabel").text(historyListJson.histories[historyIndex].user);
  $("#commentModifyTextbox").val(historyListJson.histories[historyIndex].comment);
  window.location.href="#historyDetailPage";
}

function updateSong()
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function(){
    $.mobile.loading('hide');
    $("#updateSongPage").dialog("close");
  });
  xhr.open("POST", window.origin);
  xhr.send("A");
  $.mobile.loading('show');
}
function addBook()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  var pause = "0";
  if ($("#pauseAddBookCheckbox").prop("checked") == true)
  {
    pause = "1";
  }
  var secret = "1";
  if ($("#secretAddBookCheckbox").prop("checked") == true)
  {
    secret = "0";
  }
  xhr.send(
    "5" + 
    userId + "," + 
    fileId + "," +
    $("#audioAddBookSelect").val() +
    "-2," +
    pause +
    secret +
    $("#commentAddBookTextbox").val());
  $("#addBookConfirmPage").dialog("close");
}

function insertBook()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  var pause = "0";
  if ($("#pauseInsertBookCheckbox").prop("checked") == true)
  {
    pause = "1";
  }
  var secret = "1";
  if ($("#secretInsertBookCheckbox").prop("checked") == true)
  {
    secret = "0";
  }
  xhr.send(
    "5" + 
    userId + "," + 
    fileId + "," +
    $("#audioInsertBookSelect").val() +
    "-1," +
    pause +
    secret +
    $("#commentInsertBookTextbox").val());
  $("#insertBookConfirmPage").dialog("close");
}

function detailBook(bookId, userName, path, duration, comment)
{
  $("#loginUserNameBookDetailLabel").text($("#userNameTextbox").val());
  $("#userNameBookDetailLabel").text(userName);
  $("#pathBookDetailLabel").text(path);
  var hour = ('00' + String(Math.floor(duration / 3600))).slice(-2);
  var min = ('00' + String(Math.floor((duration % 3600) / 60))).slice(-2);
  var sec = ('00' + String(duration % 60)).slice(-2);
  $("#durationBookDetailLabel").text(hour + ":" + min + ":" + sec);
  $("#commentBookDetailLabel").text(comment);
  window.bookId = bookId;
  window.location.href="#bookDetailPage";
}

function deleteBook()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("7"+bookId);
  playList();
}

function addBreak()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("50,0,0-2,11");
  $("#addBreakConfirmPage").dialog("close");
  playList();
}

function insertBreak()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("50,0,0-1,11");
  $("#insertBreakConfirmPage").dialog("close");
  playList();
}

function moveUpBook()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("80" + bookId);
  playList();
}

function moveDownBook()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("81" + bookId);
  playList();
}

function searchForm()
{
  window.location.href="#searchPage";
}

function logout()
{
  window.location.href="#loginPage";
}

function shutdown()
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function(){
    $.mobile.loading('hide');
    window.location.href="#loginPage";
  });
  xhr.open("POST", window.origin);
  xhr.send("L0" + bookId);
  $.mobile.loading('show');
}

function reboot()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("L1" + bookId);
  window.location.href="#loginPage";
}

function resetHistory()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("E" + bookId);
  window.location.href="#loginPage";
}

function addFavorite()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("J" + fileId + "," + userId);
  $("#addFavoriteConfirmPage").dialog("close");
}

function upVol()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("T1");
  window.location.href="#remoconPage";
}

function downVol()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("T0");
  window.location.href="#remoconPage";
}

function rew()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("V0");
  window.location.href="#remoconPage";
}

function ff()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("V1");
  window.location.href="#remoconPage";
}

function switchAudio()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("U");
  window.location.href="#remoconPage";
}

function pause()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("S");
  window.location.href="#remoconPage";
}

function stop()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("R");
  window.location.href="#remoconPage";
}

function deleteHistory()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("F" + historyId);
  historyList(1);
}

function modifyHistory()
{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", window.origin);
  xhr.send("G" + historyId + "," + $("#commentModifyTextbox").val());
  historyList(1);
}

function downloadHistory()
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function()
  {
    var day = new Date();
    var bom = new Uint8Array([0xEF, 0xBB, 0xBF]);
    var link = document.createElement("a");
    var blob = new Blob([bom, this.responseText], {"type" : "text/csv"});
    link.download = 
      "history_" +
      day.getFullYear() +
      String(Number(Number(day.getMonth())+1)) +
      day.getDate() +
      day.getHours() +
      day.getMinutes() + 
      day.getSeconds() +
      ".csv";
    link.href = window.URL.createObjectURL(blob);
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  });
  xhr.open("POST", window.origin);
  xhr.send("D" + historyId);
  $("#downloadHistoryConfirmPage").dialog("close");
}
function toast(message) {
    var $toast = $('<div class="ui-loader ui-overlay-shadow ui-body-e ui-corner-all"><h3>' + message + '</h3></div>');

    $toast.css({
        display: 'block', 
        background: '#fff',
        opacity: 0.90, 
        position: 'fixed',
        padding: '7px',
        'text-align': 'center',
        width: '270px',
        left: ($(window).width() - 284) / 2,
        top: $(window).height() / 2 - 20
    });

    var removeToast = function(){
        $(this).remove();
    };

    $toast.click(removeToast);

    $toast.appendTo($.mobile.pageContainer).delay(2000);
    $toast.fadeOut(400, removeToast);
}
function changeAPConfig()
{
  if (true == $("#aPConfigEnableRadio").prop("checked"))
  {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", window.origin);
    if ($("#aPConfigTextbox").val().length < 8)
    {
      alert(locale[language]["aPConfigAlert"]);
    }
    else
    {
      if (false == aPEnable)
      {
        xhr.send("Z1" + $("#aPConfigTextbox").val());
        aPEnable = true;
        aPPass = $("#aPConfigTextbox").val();
        alert(locale[language]["aPConfigRebootAlert"]);
      }
      else
      {
        if ($("#aPConfigTextbox").val() != aPPass)
        {
          xhr.send("Z1" + $("#aPConfigTextbox").val());
          aPEnable = true;
          aPPass = $("#aPConfigTextbox").val();
          alert(locale[language]["aPConfigRebootAlert"]);
        }
      }
    }
  }
  else
  {
    if (true == aPEnable)
    {
      var xhr = new XMLHttpRequest();
      xhr.open("POST", window.origin);
      xhr.send("Z0");
      aPPass = "";
      alert(locale[language]["aPConfigRebootAlert"]);
    }
  }
}
function toggleAPConfig(val)
{
  if (0 == val)
  {
    $("#aPConfigTextbox").val("");
    $("#aPConfigTextbox").prop("disabled", true);
  }
  else
  {
    $("#aPConfigTextbox").prop("disabled", false);
  }
}
function getAPConfig()
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function()
  {
    var json = $.parseJSON(this.responseText);
    if (true == json.enable)
    {
      $("#aPConfigEnableRadio").checkboxradio();
      $("#aPConfigEnableRadio").prop("checked", true);
      $("#aPConfigEnableRadio").checkboxradio("refresh");
      $("#aPConfigDisableRadio").checkboxradio();
      $("#aPConfigDisableRadio").prop("checked", false);
      $("#aPConfigDisableRadio").checkboxradio("refresh");
      $("#aPConfigTextbox").textinput();
      $("#aPConfigTextbox").val(json.phrase);
      $("#aPConfigTextbox").prop("disabled", false);
      aPEnable = true;
      aPPass = json.phrase;  
    }
    else
    {
      $("#aPConfigEnableRadio").checkboxradio();
      $("#aPConfigEnableRadio").prop("checked", false);
      $("#aPConfigEnableRadio").checkboxradio("refresh");
      $("#aPConfigDisableRadio").checkboxradio();
      $("#aPConfigDisableRadio").prop("checked", true);
      $("#aPConfigDisableRadio").checkboxradio("refresh");
      $("#aPConfigTextbox").textinput();
      $("#aPConfigTextbox").val("");
      $("#aPConfigTextbox").prop("disabled", true);
      aPEnable = false;
      aPPass = "";  
    }
    window.location.href="#aPConfigPage";
  });
  xhr.open("POST", window.origin);
  xhr.send("Y");
}
function getAudioConfig()
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function()
  {
    var json = $.parseJSON(this.responseText);
    var optItem = '';
    var index = 0;
    for(var i=0;i<json.list.length;++i)
    {
      optItem += '<option value="' + json.list[i].no + '">'
	      +  json.list[i].name
	      +  '</option>';
      if (json.no == json.list[i].no)
      {
        index = i;
      } 
      audioPort = index;
    }
    $("#audioPlaySelect").empty();
    $("#audioPlaySelect").append(optItem);
    $("#audioPlaySelect").val(index);
    $("#audioPlaySelect").selectmenu();
    $("#audioPlaySelect").selectmenu('refresh', true);
    window.location.href="#audioConfigPage";
  });
  xhr.open("POST", window.origin);
  xhr.send("O");
}
function changeAudioConfig()
{
  if ($("#audioPlaySelect").val() != audioPort)
  {
    var xhr = new XMLHttpRequest();
    xhr.addEventListener("load", function()
    {
      window.location.href="#adminPage";
    });
    xhr.open("POST", window.origin);
    xhr.send("N" + String($("#audioPlaySelect").val()));
    return;
  }
  window.location.href="#adminPage";
}
function getPlayMode()
{
  var xhr = new XMLHttpRequest();
  xhr.addEventListener("load", function()
  {
    var json = $.parseJSON(this.responseText);
    if (0 == json.mode)
    {
      $("#playModeConfigRequestRadio").checkboxradio();
      $("#playModeConfigRequestRadio").prop("checked", true);
      $("#playModeConfigRequestRadio").checkboxradio("refresh");
      $("#playModeConfigShuffleRadio").checkboxradio();
      $("#playModeConfigShuffleRadio").prop("checked", false);
      $("#playModeConfigShuffleRadio").checkboxradio("refresh");
    }
    else if (1 == json.mode)
    {
      $("#playModeConfigRequestRadio").checkboxradio();
      $("#playModeConfigRequestRadio").prop("checked", false);
      $("#playModeConfigRequestRadio").checkboxradio("refresh");
      $("#playModeConfigShuffleRadio").checkboxradio();
      $("#playModeConfigShuffleRadio").prop("checked", true);
      $("#playModeConfigShuffleRadio").checkboxradio("refresh");
    }
    window.location.href="#playModeConfigPage";
  });
  xhr.open("POST", window.origin);
  xhr.send("a");
}
function changePlayMode()
{
  if (true == $("#playModeConfigRequestRadio").prop("checked"))
  {
    var xhr = new XMLHttpRequest();
    xhr.addEventListener("load", function()
    {
      window.location.href="#adminPage";
    });
    xhr.open("POST", window.origin);
    xhr.send("b0");
  }
  else if (true == $("#playModeConfigShuffleRadio").prop("checked"))
  {
    var xhr = new XMLHttpRequest();
    xhr.addEventListener("load", function()
    {
      window.location.href="#adminPage";
    });
    xhr.open("POST", window.origin);
    xhr.send("b1");
  }
}
function modifyToAscii($this)
{
  var str=$this.value;
  while(str.match(/[^A-Z^a-z\d\-]/))
  {
    str=str.replace(/[^A-Z^a-z\d\-]/,"");
  }
  $this.value=str;
}
