/*
 * OpenGL replacement program state support.
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */
#pragma once
#include <glad/gl.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace reshade::opengl
{
	struct replacement_uniform
	{
		GLint source, destination;
		GLenum type;
	};
	struct replacement_block
	{
		GLuint source, destination;
		GLenum interface_type;
	};
	struct program_replacement_state
	{
		GLuint original = 0, replacement = 0;
		std::atomic_bool alive = true;
		std::vector<replacement_uniform> uniforms;
		std::vector<replacement_block> blocks;
	};

	inline std::string program_resource_name(const GladGLContext &gl, GLuint program, GLenum type, GLuint index)
	{
		const GLenum prop = GL_NAME_LENGTH;
		GLint length = 0;
		gl.GetProgramResourceiv(program, type, index, 1, &prop, 1, nullptr, &length);
		if (length <= 1) return {};
		std::string name(static_cast<size_t>(length), '\0');
		GLsizei written = 0;
		gl.GetProgramResourceName(program, type, index, length, &written, name.data());
		name.resize(written);
		return name;
	}

	// Preserve application-selected attribute/output locations before linking.
	// Unsupported program mechanisms fail closed, leaving the original usable.
	inline bool copy_program_interfaces(const GladGLContext &gl, GLuint original, GLuint replacement)
	{
		if (!gl.VERSION_4_3 || !gl.GetProgramResourceiv || !gl.GetProgramInterfaceiv || !gl.ProgramUniform1fv) return false;
		GLint count = 0;
		gl.GetProgramiv(original, GL_PROGRAM_SEPARABLE, &count);
		if (count != 0) return false;
		gl.GetProgramiv(original, GL_TRANSFORM_FEEDBACK_VARYINGS, &count);
		if (count != 0) return false;
		for (GLenum stage : {GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER})
		{
			gl.GetProgramStageiv(original, stage, GL_ACTIVE_SUBROUTINE_UNIFORMS, &count);
			if (count != 0) return false;
		}
		for (GLenum type : {GL_PROGRAM_INPUT, GL_PROGRAM_OUTPUT})
		{
			gl.GetProgramInterfaceiv(original, type, GL_ACTIVE_RESOURCES, &count);
			for (GLuint i = 0; i < static_cast<GLuint>(count); ++i)
			{
				const auto name = program_resource_name(gl, original, type, i);
				// SPIR-V may have no names; its explicit locations remain in the module.
				if (name.empty() || name.compare(0, 3, "gl_") == 0) continue;
				const GLenum props[] = {GL_LOCATION, GL_LOCATION_INDEX};
				GLint values[2] = {-1, 0};
				gl.GetProgramResourceiv(original, type, i, type == GL_PROGRAM_OUTPUT ? 2 : 1, props, 2, nullptr, values);
				if (values[0] < 0) continue;
				if (type == GL_PROGRAM_INPUT) gl.BindAttribLocation(replacement, values[0], name.c_str());
				else gl.BindFragDataLocationIndexed(replacement, values[0], values[1], name.c_str());
			}
		}
		return true;
	}

	inline bool copy_uniform(const GladGLContext &gl, GLuint original, GLuint replacement, const replacement_uniform &u, bool validate_only = false)
	{
		GLfloat f[16] = {};
		GLdouble d[16] = {};
		GLint i[4] = {};
		GLuint v[4] = {};
		switch (u.type)
		{
		case GL_FLOAT:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniform1fv(replacement, u.destination, 1, f); }
			return true;
		case GL_FLOAT_VEC2:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniform2fv(replacement, u.destination, 1, f); }
			return true;
		case GL_FLOAT_VEC3:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniform3fv(replacement, u.destination, 1, f); }
			return true;
		case GL_FLOAT_VEC4:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniform4fv(replacement, u.destination, 1, f); }
			return true;
		case GL_DOUBLE:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniform1dv(replacement, u.destination, 1, d); }
			return true;
		case GL_DOUBLE_VEC2:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniform2dv(replacement, u.destination, 1, d); }
			return true;
		case GL_DOUBLE_VEC3:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniform3dv(replacement, u.destination, 1, d); }
			return true;
		case GL_DOUBLE_VEC4:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniform4dv(replacement, u.destination, 1, d); }
			return true;
		case GL_INT:
		case GL_BOOL:
			if (!validate_only) { gl.GetUniformiv(original, u.source, i); gl.ProgramUniform1iv(replacement, u.destination, 1, i); }
			return true;
		case GL_INT_VEC2:
		case GL_BOOL_VEC2:
			if (!validate_only) { gl.GetUniformiv(original, u.source, i); gl.ProgramUniform2iv(replacement, u.destination, 1, i); }
			return true;
		case GL_INT_VEC3:
		case GL_BOOL_VEC3:
			if (!validate_only) { gl.GetUniformiv(original, u.source, i); gl.ProgramUniform3iv(replacement, u.destination, 1, i); }
			return true;
		case GL_INT_VEC4:
		case GL_BOOL_VEC4:
			if (!validate_only) { gl.GetUniformiv(original, u.source, i); gl.ProgramUniform4iv(replacement, u.destination, 1, i); }
			return true;
		case GL_UNSIGNED_INT:
			if (!validate_only) { gl.GetUniformuiv(original, u.source, v); gl.ProgramUniform1uiv(replacement, u.destination, 1, v); }
			return true;
		case GL_UNSIGNED_INT_VEC2:
			if (!validate_only) { gl.GetUniformuiv(original, u.source, v); gl.ProgramUniform2uiv(replacement, u.destination, 1, v); }
			return true;
		case GL_UNSIGNED_INT_VEC3:
			if (!validate_only) { gl.GetUniformuiv(original, u.source, v); gl.ProgramUniform3uiv(replacement, u.destination, 1, v); }
			return true;
		case GL_UNSIGNED_INT_VEC4:
			if (!validate_only) { gl.GetUniformuiv(original, u.source, v); gl.ProgramUniform4uiv(replacement, u.destination, 1, v); }
			return true;
		case GL_FLOAT_MAT2:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniformMatrix2fv(replacement, u.destination, 1, GL_FALSE, f); }
			return true;
		case GL_FLOAT_MAT3:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniformMatrix3fv(replacement, u.destination, 1, GL_FALSE, f); }
			return true;
		case GL_FLOAT_MAT4:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniformMatrix4fv(replacement, u.destination, 1, GL_FALSE, f); }
			return true;
		case GL_FLOAT_MAT2x3:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniformMatrix2x3fv(replacement, u.destination, 1, GL_FALSE, f); }
			return true;
		case GL_FLOAT_MAT2x4:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniformMatrix2x4fv(replacement, u.destination, 1, GL_FALSE, f); }
			return true;
		case GL_FLOAT_MAT3x2:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniformMatrix3x2fv(replacement, u.destination, 1, GL_FALSE, f); }
			return true;
		case GL_FLOAT_MAT3x4:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniformMatrix3x4fv(replacement, u.destination, 1, GL_FALSE, f); }
			return true;
		case GL_FLOAT_MAT4x2:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniformMatrix4x2fv(replacement, u.destination, 1, GL_FALSE, f); }
			return true;
		case GL_FLOAT_MAT4x3:
			if (!validate_only) { gl.GetUniformfv(original, u.source, f); gl.ProgramUniformMatrix4x3fv(replacement, u.destination, 1, GL_FALSE, f); }
			return true;
		case GL_DOUBLE_MAT2:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniformMatrix2dv(replacement, u.destination, 1, GL_FALSE, d); }
			return true;
		case GL_DOUBLE_MAT3:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniformMatrix3dv(replacement, u.destination, 1, GL_FALSE, d); }
			return true;
		case GL_DOUBLE_MAT4:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniformMatrix4dv(replacement, u.destination, 1, GL_FALSE, d); }
			return true;
		case GL_DOUBLE_MAT2x3:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniformMatrix2x3dv(replacement, u.destination, 1, GL_FALSE, d); }
			return true;
		case GL_DOUBLE_MAT2x4:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniformMatrix2x4dv(replacement, u.destination, 1, GL_FALSE, d); }
			return true;
		case GL_DOUBLE_MAT3x2:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniformMatrix3x2dv(replacement, u.destination, 1, GL_FALSE, d); }
			return true;
		case GL_DOUBLE_MAT3x4:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniformMatrix3x4dv(replacement, u.destination, 1, GL_FALSE, d); }
			return true;
		case GL_DOUBLE_MAT4x2:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniformMatrix4x2dv(replacement, u.destination, 1, GL_FALSE, d); }
			return true;
		case GL_DOUBLE_MAT4x3:
			if (!validate_only) { gl.GetUniformdv(original, u.source, d); gl.ProgramUniformMatrix4x3dv(replacement, u.destination, 1, GL_FALSE, d); }
			return true;
		case GL_SAMPLER_1D:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_1D_SHADOW:
		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_1D_ARRAY:
		case GL_SAMPLER_2D_ARRAY:
		case GL_SAMPLER_1D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_MULTISAMPLE:
		case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_SAMPLER_CUBE_SHADOW:
		case GL_SAMPLER_BUFFER:
		case GL_SAMPLER_2D_RECT:
		case GL_SAMPLER_2D_RECT_SHADOW:
		case GL_SAMPLER_CUBE_MAP_ARRAY:
		case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
		case GL_INT_SAMPLER_1D:
		case GL_INT_SAMPLER_2D:
		case GL_INT_SAMPLER_3D:
		case GL_INT_SAMPLER_CUBE:
		case GL_INT_SAMPLER_1D_ARRAY:
		case GL_INT_SAMPLER_2D_ARRAY:
		case GL_INT_SAMPLER_2D_MULTISAMPLE:
		case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_INT_SAMPLER_BUFFER:
		case GL_INT_SAMPLER_2D_RECT:
		case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_1D:
		case GL_UNSIGNED_INT_SAMPLER_2D:
		case GL_UNSIGNED_INT_SAMPLER_3D:
		case GL_UNSIGNED_INT_SAMPLER_CUBE:
		case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_BUFFER:
		case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
		case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
		case GL_IMAGE_1D:
		case GL_IMAGE_2D:
		case GL_IMAGE_3D:
		case GL_IMAGE_2D_RECT:
		case GL_IMAGE_CUBE:
		case GL_IMAGE_BUFFER:
		case GL_IMAGE_1D_ARRAY:
		case GL_IMAGE_2D_ARRAY:
		case GL_IMAGE_CUBE_MAP_ARRAY:
		case GL_IMAGE_2D_MULTISAMPLE:
		case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
		case GL_INT_IMAGE_1D:
		case GL_INT_IMAGE_2D:
		case GL_INT_IMAGE_3D:
		case GL_INT_IMAGE_2D_RECT:
		case GL_INT_IMAGE_CUBE:
		case GL_INT_IMAGE_BUFFER:
		case GL_INT_IMAGE_1D_ARRAY:
		case GL_INT_IMAGE_2D_ARRAY:
		case GL_INT_IMAGE_CUBE_MAP_ARRAY:
		case GL_INT_IMAGE_2D_MULTISAMPLE:
		case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
		case GL_UNSIGNED_INT_IMAGE_1D:
		case GL_UNSIGNED_INT_IMAGE_2D:
		case GL_UNSIGNED_INT_IMAGE_3D:
		case GL_UNSIGNED_INT_IMAGE_2D_RECT:
		case GL_UNSIGNED_INT_IMAGE_CUBE:
		case GL_UNSIGNED_INT_IMAGE_BUFFER:
		case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
		case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
		case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
		case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
		case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
			if (!validate_only) { gl.GetUniformiv(original, u.source, i); gl.ProgramUniform1iv(replacement, u.destination, 1, i); }
			return true;
		default:
			return false;
		}
	}

	inline bool build_program_state(const GladGLContext &gl, program_replacement_state *state)
	{
		GLint count = 0;
		gl.GetProgramInterfaceiv(state->replacement, GL_UNIFORM, GL_ACTIVE_RESOURCES, &count);
		for (GLuint index = 0; index < static_cast<GLuint>(count); ++index)
		{
			const GLenum props[] = {GL_LOCATION, GL_TYPE, GL_ARRAY_SIZE, GL_BLOCK_INDEX};
			GLint info[4] = {};
			gl.GetProgramResourceiv(state->replacement, GL_UNIFORM, index, 4, props, 4, nullptr, info);
			if (info[0] < 0 || info[3] != -1) continue;
			const auto name = program_resource_name(gl, state->replacement, GL_UNIFORM, index);
			if (name.compare(0, 3, "gl_") == 0) continue;
			GLuint original_index = name.empty() ? GL_INVALID_INDEX : gl.GetProgramResourceIndex(state->original, GL_UNIFORM, name.c_str());
			// Unnamed SPIR-V resources carry explicit locations. Match location AND type.
			if (name.empty())
			{
				GLint original_count = 0;
				gl.GetProgramInterfaceiv(state->original, GL_UNIFORM, GL_ACTIVE_RESOURCES, &original_count);
				for (GLuint j = 0; j < static_cast<GLuint>(original_count); ++j)
				{
					GLint candidate[4] = {};
					gl.GetProgramResourceiv(state->original, GL_UNIFORM, j, 4, props, 4, nullptr, candidate);
					if (candidate[0] == info[0] && candidate[3] == -1) { original_index = j; break; }
				}
			}
			if (original_index == GL_INVALID_INDEX) continue; // New replacement-only uniform.
			GLint original_info[4] = {};
			gl.GetProgramResourceiv(state->original, GL_UNIFORM, original_index, 4, props, 4, nullptr, original_info);
			if (original_info[1] != info[1] || original_info[3] != -1) return false;
			for (GLint element = 0; element < std::min(info[2], original_info[2]); ++element)
			{
				std::string element_name = name;
				if (info[2] > 1 && !name.empty())
				{
					const auto at = element_name.find("[0]");
					if (at == std::string::npos) return false;
					element_name.replace(at, 3, "[" + std::to_string(element) + "]");
				}
				replacement_uniform uniform = {
					name.empty() ? original_info[0] + element : gl.GetUniformLocation(state->original, element_name.c_str()),
					name.empty() ? info[0] + element : gl.GetUniformLocation(state->replacement, element_name.c_str()),
					static_cast<GLenum>(info[1])};
				if (uniform.source < 0 || uniform.destination < 0) continue;
				if (!copy_uniform(gl, state->original, state->replacement, uniform, true)) return false;
				state->uniforms.push_back(uniform);
			}
		}
		for (GLenum type : {GL_UNIFORM_BLOCK, GL_SHADER_STORAGE_BLOCK})
		{
			gl.GetProgramInterfaceiv(state->replacement, type, GL_ACTIVE_RESOURCES, &count);
			for (GLuint index = 0; index < static_cast<GLuint>(count); ++index)
			{
				const auto name = program_resource_name(gl, state->replacement, type, index);
				GLuint source = name.empty() ? GL_INVALID_INDEX : gl.GetProgramResourceIndex(state->original, type, name.c_str());
				if (name.empty())
				{
					const GLenum prop = GL_BUFFER_BINDING;
					GLint binding = 0, original_count = 0;
					gl.GetProgramResourceiv(state->replacement, type, index, 1, &prop, 1, nullptr, &binding);
					gl.GetProgramInterfaceiv(state->original, type, GL_ACTIVE_RESOURCES, &original_count);
					for (GLuint j = 0; j < static_cast<GLuint>(original_count); ++j)
					{
						GLint other = 0;
						gl.GetProgramResourceiv(state->original, type, j, 1, &prop, 1, nullptr, &other);
						if (other != binding) continue;
						if (source != GL_INVALID_INDEX) return false; // Ambiguous unnamed blocks.
						source = j;
					}
				}
				if (source != GL_INVALID_INDEX) state->blocks.push_back({source, index, type});
			}
		}
		return true;
	}

	inline void copy_program_state(const GladGLContext &gl, const program_replacement_state &state)
	{
		for (const auto &uniform : state.uniforms) copy_uniform(gl, state.original, state.replacement, uniform);
		for (const auto &block : state.blocks)
		{
			const GLenum prop = GL_BUFFER_BINDING;
			GLint binding = 0;
			gl.GetProgramResourceiv(state.original, block.interface_type, block.source, 1, &prop, 1, nullptr, &binding);
			if (block.interface_type == GL_UNIFORM_BLOCK) gl.UniformBlockBinding(state.replacement, block.destination, binding);
			else gl.ShaderStorageBlockBinding(state.replacement, block.destination, binding);
		}
	}

	// Lives only around the native draw/dispatch, never across game uniform updates.
	class program_replacement_scope
	{
		const GladGLContext *_gl = nullptr;
		GLint _previous = 0;
	public:
		program_replacement_scope(const GladGLContext &gl, const std::shared_ptr<program_replacement_state> &state)
		{
			if (!state || !state->alive.load()) return;
			gl.GetIntegerv(GL_CURRENT_PROGRAM, &_previous);
			if (static_cast<GLuint>(_previous) != state->original) return;
			copy_program_state(gl, *state);
			gl.UseProgram(state->replacement);
			_gl = &gl;
		}
		program_replacement_scope(const program_replacement_scope &) = delete;
		program_replacement_scope &operator=(const program_replacement_scope &) = delete;
		~program_replacement_scope() { if (_gl) _gl->UseProgram(_previous); }
	};
}
