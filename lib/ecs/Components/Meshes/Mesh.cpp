//#include "Mesh.hpp"
//
//namespace std
//{
//	inline void hash_combine(std::size_t& seed) { }
//
//	template <typename T, typename... Rest>
//	inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
//		std::hash<T> hasher;
//		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
//		hash_combine(seed, rest...);
//	}
//
//	template <>
//	struct hash<Components::Meshes::Mesh::Vertex>
//	{
//		size_t operator()(const Components::Meshes::Mesh::Vertex& k) const
//		{
//			std::size_t h = 0;
//			hash_combine(h, k.point, k.color, k.normal, k.texcoord);
//			return h;
//		}
//	};
//}