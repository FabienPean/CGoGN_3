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

#include <cgogn/rendering/shaders/shader_no_illum_scalar_per_vertex.h>

namespace cgogn
{

namespace rendering
{

ShaderNoIllumScalarPerVertex* ShaderNoIllumScalarPerVertex::instance_ = nullptr;

ShaderNoIllumScalarPerVertex::ShaderNoIllumScalarPerVertex()
{
	const char* vertex_shader_source = R"(
		#version 150
		uniform mat4 projection_matrix;
		uniform mat4 model_view_matrix;

		in vec3 vertex_position;
		in float vertex_scalar;
		
		out vec3 position;
		out vec3 color;

		//_insert_colormap_funcion_here

		void main()
		{
			vec4 position4 = model_view_matrix * vec4(vertex_position, 1.0);
			position = position4.xyz;
			color = scalar2color(vertex_scalar);
			gl_Position = projection_matrix * position4;
		}
	)";

	const char* fragment_shader_source = R"(
		#version 150
		uniform bool double_side;
		
		in vec3 position;
		in vec3 color;
		
		out vec3 frag_out;
		
		void main()
		{
			if (double_side || gl_FrontFacing)
				frag_out = color;
			else
				discard;
		}
	)";

	std::string v_src(vertex_shader_source);
	v_src.insert(v_src.find("//_insert_colormap_funcion_here"), shader_function::ColorMap::source);
	load2_bind(v_src, fragment_shader_source, "vertex_position", "vertex_scalar");

	get_uniforms("double_side", shader_function::ColorMap::uniform_names[0],
				 shader_function::ColorMap::uniform_names[1], shader_function::ColorMap::uniform_names[2],
				 shader_function::ColorMap::uniform_names[3]);
}

void ShaderParamNoIllumScalarPerVertex::set_uniforms()
{
	shader_->set_uniforms_values(double_side_, color_map_.color_map_, color_map_.expansion_, color_map_.min_value_,
								 color_map_.max_value_);
}

} // namespace rendering

} // namespace cgogn
