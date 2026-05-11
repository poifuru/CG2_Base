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

# ---オペレータをここに書き溜めていく---
# オペレータ 頂点を伸ばす
class MYADDON_OT_stretch_vertex(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_stretch_vertex"
    bl_label = "頂点を伸ばす"
    bl_description = "頂点座標を引っ張って伸ばします"
    # リドゥ、アンドゥ可能オプション
    bl_options = {'REGISTER', 'UNDO'}

    # メニューを実行したときに呼ばれるコールバック関数
    def execute(self, context):
        bpy.data.objects["Cube"].data.vertices[0].co.x += 1.0
        print("頂点を伸ばしました。")

        # オペレータの命令終了を通知
        return {'FINISHED'}
    
# オペレータ ICO球生成
class MYADDON_OT_create_ico_sphere(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_create_object"
    bl_label = "ICO球生成"
    bl_description = "ICO球を生成します"
    bl_options = {'REGISTER', 'UNDO'}

    # メニューを実行したときに呼ばれる関数
    def execute(self, context):
        bpy.ops.mesh.primitive_ico_sphere_add()
        print("ICO球を生成しました。")

        return {'FINISHED'}

# メニューの中身を定義するクラス
class LEVEL_EDITOR_MT_main_menu(bpy.types.Menu):
    # Blenderがクラスを識別するための固有の文字列
    bl_label = "My menu"
    # メニューのラベルとして表示される文字列
    bl_idname = "LEVEL_EDITOR_MT_main_menu"
    # 著者表示用の文字列
    bl_desctiption = "拡張メニュー by" + bl_info["author"]

    # プルダウンメニューの中身
    def draw(self, context):
        layout = self.layout
        # ここにボタンを追加していく
        layout.operator("wm.url_open", text = "公式ドキュメントを開く").url = "https://example.com"
        layout.operator("wm.url_open", text = "チュートリアル")

        layout.separator()
        layout.operator(MYADDON_OT_stretch_vertex.bl_idname, text = MYADDON_OT_stretch_vertex.bl_label)
        layout.operator(MYADDON_OT_create_ico_sphere.bl_idname, text = MYADDON_OT_create_ico_sphere.bl_label)

    def submenu(self, context):
        # 自分の bl_idname を指定してメニューとして登録する
        self.layout.menu(LEVEL_EDITOR_MT_main_menu.bl_idname)

# 登録するクラスのリスト
classes = (
    MYADDON_OT_stretch_vertex,
    MYADDON_OT_create_ico_sphere,
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