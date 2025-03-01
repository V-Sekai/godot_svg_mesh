/*************************************************************************/
/*  vg_mesh_renderer.h                                                   */
/*************************************************************************/

#ifndef VG_MESH_RENDERER_H
#define VG_MESH_RENDERER_H

#include "scene/resources/image_texture.h"
#include "utils.h"
#include "vector_graphics_renderer.h"

class VGAbstractMeshRenderer : public VGRenderer {
protected:
	tove::TesselatorRef tesselator;

	static void _bind_methods();

public:
	VGAbstractMeshRenderer();

	const tove::TesselatorRef &get_tesselator() const {
		return tesselator;
	}

	virtual Rect2 render_mesh(Ref<ArrayMesh> &p_mesh, Ref<Material> &r_material, Ref<Texture> &r_texture, VGPath *p_path, bool p_hq, bool p_spatial = false);
	virtual Ref<ImageTexture> render_texture(VGPath *p_path, bool p_hq);

	virtual bool is_dirty_on_transform_change() const {
		return false;
	}
};

#endif // VG_MESH_RENDERER_H
