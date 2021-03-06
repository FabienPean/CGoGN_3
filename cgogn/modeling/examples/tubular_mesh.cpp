/*******************************************************************************
 * CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
 * Copyright (C), IGG Group, ICube, University of Strasbourg, France            *
 *                                                                              *
 * This library is free software; you can redistribute it and/or modify it      *
 * under the terms of the GNU Lesser General Public License as published by the *
 * Free Software Foundation; either version 2.1 of the License, or (at your     *
 * option) any later version.                                                   *
 *                                                                              *
 * This library is distributed in the hope that it will be useful, but WITHOUT  *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
 * for more details.                                                            *
 *                                                                              *
 * You should have received a copy of the GNU Lesser General Public License     *
 * along with this library; if not, write to the Free Software Foundation,      *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
 *                                                                              *
 * Web site: http://cgogn.unistra.fr/                                           *
 * Contact information: cgogn@unistra.fr                                        *
 *                                                                              *
 *******************************************************************************/

#include <cgogn/core/types/mesh_traits.h>
#include <cgogn/geometry/types/vector_traits.h>

#include <cgogn/ui/app.h>
#include <cgogn/ui/view.h>

#include <cgogn/ui/modules/graph_render/graph_render.h>
#include <cgogn/ui/modules/mesh_provider/mesh_provider.h>
#include <cgogn/ui/modules/surface_render/surface_render.h>
#include <cgogn/ui/modules/volume_render/volume_render.h>

#include <cgogn/modeling/examples/tubular_mesh_module.h>

#include <cgogn/core/utils/string.h>

using Graph = cgogn::Graph;
using Surface = cgogn::CMap2;
using Volume = cgogn::CMap3;

template <typename T>
using GraphAttribute = typename cgogn::mesh_traits<Graph>::Attribute<T>;
template <typename T>
using SurfaceAttribute = typename cgogn::mesh_traits<Surface>::Attribute<T>;
template <typename T>
using VolumeAttribute = typename cgogn::mesh_traits<Volume>::Attribute<T>;

using Vec3 = cgogn::geometry::Vec3;
using Scalar = cgogn::geometry::Scalar;

int main(int argc, char** argv)
{
	std::string graph_filename, surface_filename;
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " graph_file [enclosing_surface_file]" << std::endl;
		return 1;
	}
	graph_filename = std::string(argv[1]);
	if (argc >= 3)
		surface_filename = std::string(argv[2]);

	cgogn::thread_start();

	cgogn::ui::App app;
	app.set_window_title("Tubular mesh");
	app.set_window_size(1000, 800);

	cgogn::ui::MeshProvider<Graph> mpg(app);
	cgogn::ui::MeshProvider<Surface> mps(app);
	cgogn::ui::MeshProvider<Volume> mpv(app);

	cgogn::ui::GraphRender<Graph> gr(app);
	cgogn::ui::SurfaceRender<Surface> sr(app);
	cgogn::ui::VolumeRender<Volume> vr(app);
	cgogn::ui::TubularMesh<Graph, Surface, Volume> tm(app);

	app.init_modules();

	cgogn::ui::View* v = app.current_view();

	v->link_module(&mpg);
	v->link_module(&mps);
	v->link_module(&mpv);

	v->link_module(&gr);
	v->link_module(&sr);
	v->link_module(&vr);
	v->link_module(&tm);

	// load graph
	Graph* g = mpg.load_graph_from_file(graph_filename);
	if (!g)
	{
		std::cout << "Graph file could not be loaded" << std::endl;
		return 1;
	}

	// load surface
	if (argc >= 3)
	{
		Surface* s = mps.load_surface_from_file(surface_filename);
		if (!s)
		{
			std::cout << "Surface file could not be loaded" << std::endl;
			return 1;
		}
	}

	return app.launch();
}
