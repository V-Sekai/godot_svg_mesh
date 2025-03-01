/*************************************************************************/
/*  vg_mesh_renderer.cpp                                                 */
/*************************************************************************/

#include "vector_graphics_mesh_renderer.h"
#include "modules/svg_mesh/thirdparty/tove2d/src/cpp/mesh/meshifier.h"
#include "vector_graphics_path.h"

class Renderer {
	int fill_index;
	int line_index;

	tove::MeshRef tove_mesh;
	tove::GraphicsRef root_graphics;

public:
	Renderer(const tove::MeshRef &p_tove_mesh, const tove::GraphicsRef &p_root_graphics) {
		fill_index = 0;
		line_index = 0;
		tove_mesh = p_tove_mesh;
		root_graphics = p_root_graphics;
	}

	void traverse(Node *p_node, const Transform2D &p_transform) {
		const int n = p_node->get_child_count();
		for (int i = 0; i < n; i++) {
			Node *child = p_node->get_child(i);

			Transform2D t;
			if (child->is_class_ptr(CanvasItem::get_class_ptr_static())) {
				t = p_transform * Object::cast_to<CanvasItem>(child)->get_transform();
			} else {
				t = p_transform;
			}

			traverse(child, t);
		}

		if (p_node->is_class_ptr(VGPath::get_class_ptr_static())) {
			VGPath *path = Object::cast_to<VGPath>(p_node);
			Ref<VGRenderer> renderer = path->get_inherited_renderer();
			if (renderer.is_valid() && renderer->is_class_ptr(VGAbstractMeshRenderer::get_class_ptr_static())) {
				Ref<VGAbstractMeshRenderer> meshRenderer = Object::cast_to<VGAbstractMeshRenderer>(renderer.ptr());
				if (meshRenderer.is_valid()) {
					tove::TesselatorRef tesselator = meshRenderer->get_tesselator();
					if (tesselator) {
						Size2 s = path->get_global_transform().get_scale();
						tesselator->beginTesselate(root_graphics.get(), MAX(s.width, s.height));

						tesselator->pathToMesh(
								UPDATE_MESH_EVERYTHING,
								new_transformed_path(path->get_tove_path(), p_transform),
								tove_mesh, tove_mesh,
								fill_index, line_index);

						tesselator->endTesselate();
					}
				}
			}
		}
	}
};

VGAbstractMeshRenderer::VGAbstractMeshRenderer() {
}

Rect2 VGAbstractMeshRenderer::render_mesh(Ref<ArrayMesh> &p_mesh, Ref<Material> &r_material, Ref<Texture> &r_texture, VGPath *p_path, bool p_hq, bool p_spatial) {

	clear_mesh(p_mesh);

	VGPath *root = p_path->get_root_path();
	tove::GraphicsRef subtree_graphics = root->get_subtree_graphics();

	tove::MeshRef tove_mesh;

	if (p_hq && !subtree_graphics->areColorsSolid()) {
		tove_mesh = tove::tove_make_shared<tove::PaintMesh>();
	} else {
		tove_mesh = tove::tove_make_shared<tove::ColorMesh>();
	}

	Renderer r(tove_mesh, subtree_graphics);
	r.traverse(p_path, Transform2D());

	r_material = copy_mesh(p_mesh, tove_mesh, subtree_graphics, r_texture, p_spatial);

	return tove_bounds_to_rect2(p_path->get_tove_path()->getBounds());
}

Ref<ImageTexture> VGAbstractMeshRenderer::render_texture(VGPath *p_path, bool p_hq) {
	return Ref<ImageTexture>();
}
