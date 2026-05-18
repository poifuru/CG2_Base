import bpy
import bpy_extras
import math

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
    bl_description = "ICO球を生成するよ"
    bl_options = {'REGISTER', 'UNDO'}

    # メニューを実行したときに呼ばれる関数
    def execute(self, context):
        bpy.ops.mesh.primitive_ico_sphere_add()
        print("ICO球を生成しました。")

        return {'FINISHED'}

# オペレータ シーン出力
class MYADDON_OT_export_scene(bpy.types.Operator, bpy_extras.io_utils.ExportHelper):
    bl_idname = "myaddon.myaddon_ot_export_scene"
    bl_label = "シーン出力"
    bl_description = "シーン情報をExportします"
    # 出力するファイルの拡張子
    filename_ext = ".scene"

    def export(self):
        """ファイルに出力"""

        print("シーン情報出力開始... %r" % self.filepath)

        # ファイルをテキスト形式で書き出し用にオープン
        # スコープを抜けると自動的にクローズされる
        with open(self.filepath, "wt") as file:

            # 最初にシーン内のオブジェクト総数を出力
            self.write_and_print(file, "SCENE : " + f"{len(bpy.context.scene.objects)}")

            # シーン内の全オブジェクトについて
            for object in bpy.context.scene.objects:
                # 親オブジェクトがあるものはスキップ(親から呼び出すため)
                if(object.parent):
                    continue

                # シーン直下のオブジェクトをルートノード(深さ0)とし、再起関数で走査
                self.parse_scene_recursive(file, object, 0)

    def execute(self, context):

        print("シーン情報をExportします")

        # ファイルに出力
        self.export()

        print("シーン情報をExportしました")
        self.report({'INFO'}, "シーン情報をExportしました")

        return {'FINISHED'}
    
    def write_and_print(self, file, str):
        print(str)

        file.write(str)
        file.write('\n')

    def parse_scene_recursive(self, file, object, level):
        """シーン解析用再起関数"""

        # 深さ分インデント(タブを挿入する)
        indent = ''
        for i in range(level):
            indent += "\t"

        # オブジェクト名書き込み
        self.write_and_print(file, indent + object.type + " - " + object.name)
        trans, rot, scale = object.matrix_local.decompose()
        # 回転を Quternion から Euler (3軸での回転角) に変更
        rot = rot.to_euler()
        # ラジアンから度数法に変換
        rot.x = math.degrees(rot.x)
        rot.y = math.degrees(rot.y)
        rot.z = math.degrees(rot.z)
        # トランスフォーム情報を表示
        self.write_and_print(file, indent + "Trans(%f,%f,%f)" % (trans.x, trans.y, trans.z) )
        self.write_and_print(file, indent + "Rot(%f,%f,%f)" % (rot.x, rot.y, rot.z) )
        self.write_and_print(file, indent + "Scale(%f,%f,%f)" % (scale.x, scale.y, scale.z) )
        self.write_and_print(file, '')

        # 子ノードへ進む(深さが1上がる)
        for child in object.children:
            self.parse_scene_recursive(file, child, level + 1)

# メニューの中身を定義するクラス
class LEVEL_EDITOR_MT_main_menu(bpy.types.Menu):
    # Blenderがクラスを識別するための固有の文字列
    bl_label = "My menu"
    # メニューのラベルとして表示される文字列
    bl_idname = "LEVEL_EDITOR_MT_main_menu"
    # 著者表示用の文字列
    bl_description = "拡張メニュー by" + bl_info["author"]

    # プルダウンメニューの中身
    def draw(self, context):
        layout = self.layout
        # ここにボタンを追加していく
        layout.operator("wm.url_open", text = "公式ドキュメントを開く").url = "https://example.com"
        layout.operator("wm.url_open", text = "チュートリアル")

        layout.separator()
        layout.operator(MYADDON_OT_stretch_vertex.bl_idname, text = MYADDON_OT_stretch_vertex.bl_label)
        layout.operator(MYADDON_OT_create_ico_sphere.bl_idname, text = MYADDON_OT_create_ico_sphere.bl_label)

        layout.separator()
        layout.operator(MYADDON_OT_export_scene.bl_idname, text = MYADDON_OT_export_scene.bl_label)

    def submenu(self, context):
        # 自分の bl_idname を指定してメニューとして登録する
        self.layout.menu(LEVEL_EDITOR_MT_main_menu.bl_idname)

# 登録するクラスのリスト
classes = (
    MYADDON_OT_stretch_vertex,
    MYADDON_OT_create_ico_sphere,
    MYADDON_OT_export_scene,
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