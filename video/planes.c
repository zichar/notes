#planes



int drm_universal_plane_init(struct drm_device *dev, struct drm_plane *plane,
			     uint32_t possible_crtcs,
			     const struct drm_plane_funcs *funcs,
			     const uint32_t *formats, unsigned int format_count,
			     const uint64_t *format_modifiers,
			     enum drm_plane_type type,
			     const char *name, ...)
	__drm_universal_plane_init(dev, plane, possible_crtcs, funcs,
				formats, format_count, format_modifiers,
				type, name, ap);
		struct drm_mode_config *config = &dev->mode_config;
		drm_mode_object_add(dev, &plane->base, DRM_MODE_OBJECT_PLANE);
		plane->base.properties = &plane->properties;
		plane->dev = dev;
		plane->funcs = funcs;
		plane->format_types = kmalloc_array(format_count, sizeof(uint32_t), GFP_KERNEL);
		plane->modifier_count = format_modifier_count;
		plane->modifiers = kmalloc_array(format_modifier_count, sizeof(format_modifiers[0]), GFP_KERNEL);
		plane->name = kvasprintf(GFP_KERNEL, name, ap); /plane->name = kasprintf(GFP_KERNEL, "plane-%d", drm_num_planes(dev));
		plane->format_count = format_count;
		plane->possible_crtcs = possible_crtcs;
		plane->type = type;
		list_add_tail(&plane->head, &config->plane_list);
		plane->index = config->num_total_plane++;
		drm_object_attach_property(&plane->base, config->plane_type_property, plane->type);
		if (format_modifier_count)
			create_in_format_blob(dev, plane);


drm_plane_helper_add(plane, &linlondp_plane_helper_funcs);














