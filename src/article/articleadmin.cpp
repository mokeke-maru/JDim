// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articleadmin.h"
#include "font.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"

#include "skeleton/view.h"
#include "skeleton/dragnote.h"

#include "icons/iconmanager.h"

#include "global.h"
#include "viewfactory.h"
#include "dndmanager.h"
#include "sharedbuffer.h"
#include "session.h"
#include "command.h"


ARTICLE::ArticleAdmin *instance_articleadmin = NULL;

ARTICLE::ArticleAdmin* ARTICLE::get_admin()
{
    if( ! instance_articleadmin ) instance_articleadmin = new ARTICLE::ArticleAdmin( URL_ARTICLEADMIN );
    assert( instance_articleadmin );

    return instance_articleadmin;
}

void ARTICLE::delete_admin()
{
    if( instance_articleadmin ) delete instance_articleadmin;
    instance_articleadmin = NULL;
}


using namespace ARTICLE;

ArticleAdmin::ArticleAdmin( const std::string& url )
    : SKELETON::Admin( url )
{
    ARTICLE::init_font();

    get_notebook()->set_dragable( true );
    get_notebook()->set_fixtab( false );
    if( ! SESSION::get_show_article_tab() ) get_notebook()->set_show_tabs( false );

    setup_menu( true );
}


ArticleAdmin::~ArticleAdmin()
{
#ifdef _DEBUG    
    std::cout << "ArticleAdmin::~ArticleAdmin\n";
#endif

    SESSION::set_article_URLs( get_URLs() );
    SESSION::set_article_locked( get_locked() );
    SESSION::set_article_page( get_current_page() );
    ARTICLE::init_font();
}


// 前回開いていたURLを復元
void ArticleAdmin::restore()
{
#ifdef _DEBUG
    std::cout << "ArticleAdmin::restore\n";
#endif

    JDLIB::Regex regex;

    bool online = SESSION::is_online();
    SESSION::set_online( false );

    std::list< std::string > list_url = SESSION::get_article_URLs();
    std::list< std::string >::iterator it_url = list_url.begin();

    std::list< bool > list_locked = SESSION::get_article_locked();
    std::list< bool >::iterator it_locked = list_locked.begin();

    for( ; it_url != list_url.end(); ++it_url ){

        std::string url = (*it_url);
        COMMAND_ARGS command_arg;
        command_arg.command = "open_view";
        command_arg.url = std::string();

        // タブのロック状態
        std::string lock;
        if( it_locked != list_locked.end() ){
            if( (*it_locked ) ) lock = "lock";
            ++it_locked;
        }

        // レス抽出
        if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + RES_SIGN + "(.*)"
                        + CENTER_SIGN + "(.*)" + TIME_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = lock;

            command_arg.arg4 = "RES";
            command_arg.arg5 = regex.str( 2 );
            if( regex.str( 3 ) != "0" ) command_arg.arg6 = regex.str( 3 );
        }

        // 名前抽出
        else if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + NAME_SIGN + "(.*)" + TIME_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = lock;

            command_arg.arg4 = "NAME";
            command_arg.arg5 = regex.str( 2 );
        }

        // ID抽出
        else if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + ID_SIGN + "(.*)" + TIME_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = lock;

            command_arg.arg4 = "ID";
            command_arg.arg5 = regex.str( 2 );
        }

        // ブックマーク抽出
        else if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + BOOKMK_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = lock;

            command_arg.arg4 = "BM";
        }

        // URL抽出
        else if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + URL_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = lock;

            command_arg.arg4 = "URL";
        }

        // 参照
        else if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + REFER_SIGN + "(.*)" + TIME_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = lock;

            command_arg.arg4 = "REF";
            command_arg.arg5 = regex.str( 2 );
        }

        // キーワード
        else if( regex.exec( std::string( "(.*)" ) + ARTICLE_SIGN + KEYWORD_SIGN + "(.*)"
                             + ORMODE_SIGN + "(.*)" + TIME_SIGN, url )){

            command_arg.url = regex.str( 1 );
            command_arg.arg1 = "true"; // タブで開く
            command_arg.arg2 = "true"; // 既に開いているかチェック無し
            command_arg.arg3 = lock;

            if( regex.str( 3 ) == "1" ) command_arg.arg4 = "KEYWORD_OR";
            else command_arg.arg4 = "KEYWORD";
            command_arg.arg5 = regex.str( 2 );
        }

        // キャッシュ検索は重いので復元しない
        else if( regex.exec( std::string( "(.*)" ) + BOARD_SIGN + KEYWORD_SIGN + "(.*)"
                             + ORMODE_SIGN + "(.*)" + TIME_SIGN, url )){
        }

        // MAIN
        else if( !url.empty() ){
            command_arg.url = url;
            command_arg.arg1 = "true";   // タブで開く
            command_arg.arg2 = "false";  // 既に開いているかチェック
            command_arg.arg3 = lock;

            command_arg.arg4 = "MAIN";
        }

#ifdef _DEBUG
        std::cout << command_arg.url << std::endl
                  << command_arg.arg1 << std::endl
                  << command_arg.arg2 << std::endl
                  << command_arg.arg3 << std::endl
                  << command_arg.arg4 << std::endl
                  << command_arg.arg5 << std::endl;
#endif

        if( ! command_arg.url.empty() ) open_view( command_arg );
    }



    SESSION::set_online( online );
    set_command( "set_page", std::string(), MISC::itostr( SESSION::article_page() ) );
}


void ArticleAdmin::switch_admin()
{
    if( ! has_focus() ) CORE::core_set_command( "switch_article" );
}


//
// リストで与えられたページをタブで連続して開くとき(Admin::open_list())の引数セット
//
COMMAND_ARGS ArticleAdmin::get_open_list_args( const std::string& url )
{
    COMMAND_ARGS command_arg;
    command_arg.arg4 = "MAIN";

    CORE::core_set_command( "set_history_article", url );

    return command_arg;
}



//
// カレントビューでポップアップ表示していたら消す
//
void ArticleAdmin::delete_popup()
{
    SKELETON::View* view = get_current_view();
    if( view ) view->set_command( "delete_popup" );
}


//
// view の作成
//
SKELETON::View* ArticleAdmin::create_view( const COMMAND_ARGS& command )
{
#ifdef _DEBUG    
    std::cout << "ArticleAdmin::create_view : " << command.arg4 << std::endl;
#endif

    delete_popup();

    int type = CORE::VIEW_NONE; 
    CORE::VIEWFACTORY_ARGS view_args;
    std::string str_jump;

    // メインビュー
    if( command.arg4 == "MAIN" ){
        type = CORE::VIEW_ARTICLEVIEW;
    }

    // キーワード抽出ビュー(AND)
    else if( command.arg4 == "KEYWORD" ){
        type = CORE::VIEW_ARTICLEDRAWOUT;
        view_args.arg1 = command.arg5;  // query
        view_args.arg2 = "AND";
    }

    // キーワード抽出ビュー(OR)
    else if( command.arg4 == "KEYWORD_OR" ){
        type = CORE::VIEW_ARTICLEDRAWOUT;
        view_args.arg1 = command.arg5; // query
        view_args.arg2 = "OR";
    }

    // レス抽出ビュー
    else if( command.arg4 == "RES" ){
        type = CORE::VIEW_ARTICLERES;
        view_args.arg1 = command.arg5; // レス番号 ( from-to )
        view_args.arg2 = "false";
        view_args.arg3 = command.arg6; // ジャンプ番号
        str_jump = command.arg6;
    }

    // 名前抽出ビュー
    else if( command.arg4 == "NAME" ){
        type = CORE::VIEW_ARTICLENAME;
        view_args.arg1 = command.arg5; // 名前
    }

    // ID 抽出ビュー
    else if( command.arg4 == "ID" ){
        type = CORE::VIEW_ARTICLEID;
        view_args.arg1 = command.arg5; // ID
    }

    // ブックマーク抽出ビュー
    else if( command.arg4 == "BM" ){
        type = CORE::VIEW_ARTICLEBM;
    }

    // URL抽出ビュー
    else if( command.arg4 == "URL" ){
        type = CORE::VIEW_ARTICLEURL;
    }

    // 参照抽出ビュー
    else if( command.arg4 == "REF" ){
        type = CORE::VIEW_ARTICLEREFER;
        view_args.arg1 = command.arg5; // 対象レス番号
    }

    // ログ検索
    else if( command.arg4 == "SEARCHLOG" ){
        type = CORE::VIEW_ARTICLESEARCHLOG;
        view_args.arg1 = command.arg5;  // query
    }

    // 全キャッシュログ検索
    else if( command.arg4 == "SEARCHALLLOG" ){
        type = CORE::VIEW_ARTICLESEARCHALLLOG;
        view_args.arg1 = command.arg5;  // query
    }

    // タイトル検索
    else if( command.arg4 == "SEARCHTITLE" ){
        type = CORE::VIEW_ARTICLESEARCHTITLE;
        view_args.arg1 = command.arg5;  // query
    }

    else return NULL;

    SKELETON::View* view = CORE::ViewFactory( type, command.url, view_args );
    assert( view != NULL );

    // ジャンプ
    if( ! str_jump.empty() ){
#ifdef _DEBUG    
        std::cout << "goto " << str_jump << std::endl;
#endif
        set_command( "goto_num", view->get_url(), str_jump );
    }

    return view;
}


//
// ローカルなコマンド
//
void ArticleAdmin::command_local( const COMMAND_ARGS& command )
{
    if( command.command == "goto_num" ){
        SKELETON::View* view = get_view( command.url );
        if( view ) view->set_command( "goto_num", command.arg1 );
    }

    // ポップアップ消去
    else if( command.command == "delete_popup" ) delete_popup();

    // command.url を含むビューを全て再レイアウト
    else if( command.command == "relayout_views" ){

        std::list< SKELETON::View* > list_view = get_list_view( command.url );
        std::list< SKELETON::View* >::iterator it = list_view.begin();

        for( ; it != list_view.end(); ++it ){
            SKELETON::View* view = ( *it );
            if( view ) view->relayout();
        }
    }

    // フォント初期化
    else if( command.command == "init_font" ) ARTICLE::init_font();
}





//
// タブのD&Dを開始
//
void ArticleAdmin::slot_drag_begin( int page )
{
    SKELETON::View* view = ( SKELETON::View* )get_notebook()->get_nth_page( page );
    if( !view ) return;

    std::string url = view->get_url();
    
    CORE::DND_Begin( get_url() );

    CORE::DATA_INFO info;
    info.type = TYPE_THREAD;
    info.url = DBTREE::url_readcgi( url, 0, 0 );
    info.name = DBTREE::article_subject( info.url );

    CORE::SBUF_clear_info();
    if( info.url.empty() ) return;

#ifdef _DEBUG    
    std::cout << "ArticleAdmin::slot_drag_begin " << url  << std::endl;
    std::cout << "name = " << info.name << std::endl;
    std::cout << "url ~ " << info.url << std::endl;
    std::cout << "type  =" << info.type << std::endl;
#endif

    CORE::SBUF_append( info );
}



//
// タブのD&D終了
//
void ArticleAdmin::slot_drag_end()
{
#ifdef _DEBUG    
    std::cout << "ArticleAdmin::slot_drag_end\n";
#endif

    CORE::DND_End();
}
