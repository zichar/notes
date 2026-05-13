
#component add
static const struct component_master_ops master_ops = {
	.bind = master_bind,
	.unbind = master_unbind,
};

drm_of_component_match_add(master, match/matchptr, compare_of/compare, remote/node); //remote = of_graph_get_remote_node(np, port, endpoint);
->component_match_add_release(master, matchptr, component_release_of, compare, node/compare_data);
-->__component_match_add(parent, matchptr, release, compare, NULL, compare_data);
	if (!match)
		match = devres_alloc(devm_component_match_release, sizeof(*match), GFP_KERNEL);
		devres_add(parent, match);
		*matchptr = match;
	if (match->num == match->alloc)
		component_match_realloc(match, new_size);
	match->compare[match->num].compare = compare;
	match->compare[match->num].compare_typed = compare_typed;
	match->compare[match->num].release = release;
	match->compare[match->num].data = compare_data;
	match->compare[match->num].component = NULL;
	match->num++;

#master
component_master_add_with_match(dev, &linlondp_master_ops, match);
	component_match_realloc(match, match->num);
	adev = kzalloc(sizeof(*adev), GFP_KERNEL);
	if (!adev)
		return -ENOMEM;
	adev->parent = parent;
	adev->ops = ops;
	adev->match = match;
	component_debugfs_add(adev);
	list_add(&adev->node, &aggregate_devices);
	try_to_bring_up_aggregate_device(adev, NULL);
		find_components(adev);
			for (i = 0; i < match->num; i++) {
				c = find_component(adev, mc);
					list_for_each_entry(c, &component_list, node) {
						if (mc->compare && mc->compare(c->dev, mc->data))
							return c;
						if (mc->compare_typed && mc->compare_typed(c->dev, c->subcomponent, mc->data))
							return c;
					}
				match->compare[i].duplicate = !!c->adev;
				match->compare[i].component = c;
				c->adev = adev;
			}
		adev->ops->bind(adev->parent);

#master bind call slave bind
component_bind_all(mdev->dev, kms);
	adev = __aggregate_find(parent, NULL);
	for (i = 0; i < adev->match->num; i++)
		if (!adev->match->compare[i].duplicate)
			c = adev->match->compare[i].component;
			component_bind(c, adev, data);
				component->ops->bind(component->dev, adev->parent, data);

#slave

static const struct component_ops slave_ops = {
	.bind = slave_bind,
	.unbind = slave_unbind,
};

component_bind_all(mdev->dev, kms);
	adev = __aggregate_find(parent, NULL);
	for (i = 0; i < adev->match->num; i++)
		if (!adev->match->compare[i].duplicate)
			c = adev->match->compare[i].component;
			component_bind(c, adev, data);
				component->ops->bind(component->dev, adev->parent, data);
