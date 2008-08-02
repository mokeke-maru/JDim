// ライセンス: GPL2

// データベースのルートクラス
//
// クラス図  [ Root ] ---> [ BoardBase ] ---> [ ArticleBase ] ---> [ NodeTreeBase ]
//

#ifndef _ROOT_H
#define _ROOT_H

#include "etcboardinfo.h"

#include "skeleton/loadable.h"
#include "xml/document.h"

#include <string>
#include <list>

namespace DBTREE
{
    class BoardBase;

    // サーバ移転テーブル
    //
    // (1) bbsmenuを読み込んで移転していた場合( Root::set_board() )
    //
    // 現在のホストを新ホストに移動する。キャッシュも移動する
    //
    // (2) 参照しようとした板が無かった時( Root::get_board() )
    //
    // 参照した古いホストの移動先を現在のホストに設定する。キャッシュは移動しない
    //
    // (3) ある板のsubject.txtを読み込んだときにHTTP_REDIRECTが戻ってきて
    //     BoardBase::start_checkking_if_board_moved()により移転が確認されたとき
    // 
    // 現在のホストを新ホストに移動する。キャッシュも移動する
    //
    struct MOVETABLE
    {
        std::string old_root;
        std::string new_root;
        std::string old_path_board;
        std::string new_path_board;
    };

    
    class Root : public SKELETON::Loadable
    {
        // Boardクラス のキャッシュ
        // Boardクラスは一度作ったら~Root()以外ではdeleteしないこと
        std::list< BoardBase* > m_list_board;

        // 鯖移転テーブル
        std::list< MOVETABLE > m_movetable;

        std::string m_xml_bbsmenu;
        XML::Document m_document;
        char* m_rawdata;
        size_t m_lng_rawdata;
        std::list< DBTREE::ETCBOARDINFO > m_etcboards; // 外部板情報
        std::string m_move_info;

        // NULL board クラス
        BoardBase* m_board_null;

        // get_board()のキャッシュ
        // get_article_fromURL()のキャッシュ
        std::string m_get_board_url;
        BoardBase* m_get_board;

      public:

        Root();
        ~Root();

        // 板一覧のxml
        const std::string& xml_bbsmenu() const { return m_xml_bbsmenu; }
        const XML::Document& xml_document() const { return m_document; }

        // 板移転
        const bool move_board( const std::string& url_old, const std::string& url_new, const bool etc );

        // 外部板情報取得
        const std::list< DBTREE::ETCBOARDINFO >& get_etcboards() const { return m_etcboards; }

        // 外部板追加
        bool add_etc( const std::string& url, const std::string& name, const std::string& basicauth, const std::string& id );

        // 外部板更新
        bool move_etc( const std::string& url_old, const std::string& url_new,
                       const std::string& name_old, const std::string& name_new,
                       const std::string& basicauth, const std::string& boardid );

        // 外部板削除
        bool remove_etc( const std::string& url, const std::string& name );

        // 外部板情報保存
        void save_etc();

        // Board クラスのポインタ取得
        BoardBase* get_board( const std::string& url, const int count = 0 );

        // bbsmenuのダウンロード
        void download_bbsmenu();

        // 配下の全boardbaseクラスのレスあぼーん状態を更新する
        void update_abone_all_board();

        // 配下の全boardbaseクラスに、全articlebaseクラスのあぼーん状態の更新をさせる
        void update_abone_all_article();

        // 板が移転したかチェックする
        // 移転した時は移転後のURLを返す
        const std::string is_board_moved( const std::string& url ); // 簡易版

        const std::string is_board_moved( const std::string& url,
                                          std::string& old_root,
                                          std::string& old_path_board,
                                          std::string& new_root,
                                          std::string& new_path_board,
                                          const int count = 0
            );

        // 全板の情報ファイル読み込み
        void read_boardinfo_all();

        // 全ログ検索
        std::list< std::string > search_cache( const std::string& query, bool mode_or, bool& stop );

      private:

        // bbsmenuのダウンロード用関数
        void clear();
        virtual void receive_data( const char* data, size_t size );
        virtual void receive_finish();
        void bbsmenu2xml( const std::string& menu );

        // XML に含まれる板情報を取り出してデータベースを更新
        void analyze_board_xml();

        // 板のタイプを判定
        int get_board_type( const std::string& url, std::string& root, std::string& path_board, const bool etc );

        // 板のタイプに合わせて板情報をセット
        bool set_board( const std::string& url, const std::string& name, const std::string& basicauth = std::string(), bool etc = false );

        // 板移転処理
        bool exec_move_board( BoardBase* board,
                              const std::string old_root,
                              const std::string old_path_board,
                              const std::string new_root,
                              const std::string new_path_board
            );

        // 板移転テーブルに追加
        void push_movetable( const std::string old_root,
                             const std::string old_path_board,
                             const std::string new_root,
                             const std::string new_path_board
            );

        // 板をデータベースから削除
        bool remove_board( const std::string& url );

        int is_moved( const std::string& root,
                       const std::string& path_board,
                       const std::string& name,
                       BoardBase** board_old );

        void load_cache();
        void load_etc();
        void load_movetable();
        void save_movetable();

        // urlのタイプ判定
        bool is_2ch( const std::string& url );
        bool is_JBBS( const std::string& url );
        bool is_machi( const std::string& url );
    };
}


#endif
