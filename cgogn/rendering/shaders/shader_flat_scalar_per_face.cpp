﻿/*******************************************************************************
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

#include <cgogn/rendering/shaders/shader_flat_scalar_per_face.h>

namespace cgogn
{

namespace rendering
{

ShaderFlatScalarPerFace* ShaderFlatScalarPerFace::instance_ = nullptr;

ShaderFlatScalarPerFace::ShaderFlatScalarPerFace()
{
	const char* vertex_shader_source = R"(
		#version 330
		uniform mat4 projection_matrix;
		uniform mat4 model_view_matrix;
		
		uniform usamplerBuffer vertex_ind;
		uniform usamplerBuffer face_ind;
		uniform samplerBuffer vertex_position;
		uniform samplerBuffer face_scalar;
		
		out vec3 position;
		flat out vec3 color;

		//_insert_colormap_function_here

		void main()
		{
			int ind_v = int(texelFetch(vertex_ind, 3 * gl_InstanceID + gl_VertexID).r);
			vec3 position_in = texelFetch(vertex_position, ind_v).rgb;

			int ind_f = int(texelFetch(face_ind, int(gl_InstanceID)).r);
			float value = transform_value(texelFetch(face_scalar, ind_f).r);
			color = value2color(value);

			vec4 position4 = model_view_matrix * vec4(position_in, 1.0);
			position = position4.xyz;
			gl_Position = projection_matrix * position4;
		}
	)";

	const char* fragment_shader_source = R"(
		#version 330
		uniform vec4 ambiant_color;
		uniform vec3 light_position;
		uniform bool double_side;
		
		in vec3 position;
		flat in vec3 color;

		out vec3 frag_out;

		void main()
		{
			vec3 N = normalize(cross(dFdx(position), dFdy(position)));
			vec3 L = normalize(light_position - position);
			float lambert = dot(N, L);
			if (!gl_FrontFacing && !double_side)
				discard;
			else
				frag_out = ambiant_color.rgb + lambert * color;
		}
	)";

	std::string v_src(vertex_shader_source);
	v_src.insert(v_src.find("//_insert_colormap_function_here"), shader_function::ColorMap::source);
	load(v_src, fragment_shader_source);
	get_uniforms("vertex_ind", "face_ind", "vertex_position", "face_scalar", "ambiant_color", "light_position",
				 "double_side", shader_function::ColorMap::uniform_names[0],
				 shader_function::ColorMap::uniform_names[1], shader_function::ColorMap::uniform_names[2],
				 shader_function::ColorMap::uniform_names[3]);

	nb_attributes_ = 2;
}

void ShaderParamFlatScalarPerFace::set_uniforms()
{
	shader_->set_uniforms_values(10, 11, 12, 13, ambiant_color_, light_position_, double_side_, color_map_.color_map_,
								 color_map_.expansion_, color_map_.min_value_, color_map_.max_value_);
}

void ShaderParamFlatScalarPerFace::bind_texture_buffers()
{
	vbos_[VERTEX_POSITION]->bind_texture_buffer(12);
	vbos_[FACE_SCALAR]->bind_texture_buffer(13);
}

void ShaderParamFlatScalarPerFace::release_texture_buffers()
{
	vbos_[VERTEX_POSITION]->release_texture_buffer(12);
	vbos_[FACE_SCALAR]->release_texture_buffer(13);
}

} // namespace rendering

} // namespace cgogn
