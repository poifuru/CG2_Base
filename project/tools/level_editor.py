import bpy

# ブレンダーに登録するアドオン情報
bl_info = {
    "name": "レベルエディタ",
    "author": "Go Masuya",
    "version": (1, 0),
    "blender": (4, 4, 0),
    "location": "Top Bar > Level Editor",
    "description": "レベルエディタ",
    "category": "Object"
}

# メニューの中身を定義するクラス
class LEVEL_EDITOR_MT_main_menu(bpy.types.Menu):
    bl_label = "My menu"
    bl_idname = "LEVEL_EDITOR_MT_main_menu"
    bl_desctiption = "拡張メニュー by" + bl_info["author"]

    # プルダウンメニューの中身
    def draw(self, context):
        layout = self.layout
        # ここにボタンを追加していく
        layout.operator("wm.url_open", text = "公式ドキュメントを開く").url = "https://example.com"

        layout.separator()
        layout.operator("wm.url_open", text = "チュートリアル")

    def submenu(self, context):
        # 自分の bl_idname を指定してメニューとして登録する
        self.layout.menu(LEVEL_EDITOR_MT_main_menu.bl_idname)

# 登録するクラスのリスト
classes = (
    LEVEL_EDITOR_MT_main_menu,
)

#アドオン有効時コールバック
def register():
    # クラスの登録
    for cls in classes:
        bpy.utils.register_class(cls)

    # トップバーのメニュー(TOPBAR_MT_editor_menus)に描画関数を追加する
    bpy.types.TOPBAR_MT_editor_menus.append(LEVEL_EDITOR_MT_main_menu.submenu)
    print("レベルエディタが有効化されました。")

#アドオン無効時コールバック
def unregister():
    # 追加した描画関数を削除する
    bpy.types.TOPBAR_MT_editor_menus.remove(LEVEL_EDITOR_MT_main_menu.submenu)

    # クラスの登録解除
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
    print("レベルエディタが無効化されました。")
    
if __name__=="__main__":
    register()