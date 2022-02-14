#include "image_loader_svg_spatial.h"

ResourceImporterSVGSpatial::ResourceImporterSVGSpatial() {
}

ResourceImporterSVGSpatial::~ResourceImporterSVGSpatial() {
}

Node *ResourceImporterSVGSpatial::import_scene(const String &p_path, uint32_t p_flags, int p_bake_fps, uint32_t p_compress_flags, List<String> *r_missing_deps, Error *r_err) {
	Spatial *root = memnew(Spatial);
	String units = "px";
	float dpi = 96.0;
	Vector<uint8_t> buf = FileAccess::get_file_as_array(p_path);
	if (!buf.size()) {
		return FAILED;
	}
	String str;
	str.parse_utf8((const char *)buf.ptr(), buf.size());
	tove::GraphicsRef tove_graphics = tove::Graphics::createFromSVG(
			str.utf8().ptr(), units.utf8().ptr(), dpi);
	{
		const float *bounds = tove_graphics->getBounds();
		float s = 256.0f / MAX(bounds[2] - bounds[0], bounds[3] - bounds[1]);
		if (s > 1.0f) {
			tove::nsvg::Transform transform(s, 0, 0, 0, s, 0);
			transform.setWantsScaleLineWidth(true);
			tove_graphics->set(tove_graphics, transform);
		}
	}
	int32_t n = tove_graphics->getNumPaths();
	Ref<VGMeshRenderer> renderer;
	renderer.instance();
	renderer->set_quality(0.4);
	VGPath *root_path = memnew(VGPath(tove::tove_make_shared<tove::Path>()));
	root->add_child(root_path);
	root_path->set_owner(root);
	root_path->set_renderer(renderer);
	Ref<SurfaceTool> st;
	st.instance();
	Spatial *spatial = memnew(Spatial);
	spatial->set_name(root_path->get_name());
	root->add_child(spatial);
	spatial->set_owner(root);
	AABB bounds;
	for (int mesh_i = 0; mesh_i < n; mesh_i++) {
		tove::PathRef tove_path = tove_graphics->getPath(mesh_i);
		Point2 center = compute_center(tove_path);
		tove_path->set(tove_path, tove::nsvg::Transform(1, 0, -center.x, 0, 1, -center.y));
		VGPath *path = memnew(VGPath(tove_path));
		path->set_position(center);
		root_path->add_child(path);
		path->set_owner(root);
		Ref<ArrayMesh> mesh;
		mesh.instance();
		Ref<Texture> texture;
		Ref<Material> renderer_material;
		renderer->render_mesh(mesh, renderer_material, texture, path, true, true);
		Transform xform;
		real_t gap = mesh_i * CMP_POINT_IN_PLANE_EPSILON * 16.0f;
		MeshInstance *mesh_inst = memnew(MeshInstance);
		mesh_inst->translate(Vector3(center.x * 0.001f, -center.y * 0.001f, gap));
		if (renderer_material.is_null()) {
			Ref<SpatialMaterial> mat;
			mat.instance();
			mat->set_texture(SpatialMaterial::TEXTURE_ALBEDO, texture);
			mat->set_flag(SpatialMaterial::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
			mat->set_cull_mode(SpatialMaterial::CULL_DISABLED);
			mesh->surface_set_material(0, mat);
		} else {
			mesh->surface_set_material(0, renderer_material);
		}
		mesh_inst->set_mesh(mesh);
		bounds = bounds.merge(mesh_inst->get_aabb());
		String name = tove_path->getName();
		if (!name.empty()) {
			mesh_inst->set_name(name);
		}
		spatial->add_child(mesh_inst);
		mesh_inst->set_owner(root);
	}
	Vector3 translate = bounds.get_size();
	translate.x = -translate.x;
	translate.x += translate.x / 2.0f;
	translate.y += translate.y;
	spatial->translate(translate);
	memdelete(root_path);
	return root;
}
