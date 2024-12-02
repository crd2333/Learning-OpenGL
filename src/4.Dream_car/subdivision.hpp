#include <vector>
#include <cmath>
#include <unordered_map>

struct Point {
    float x, y, z;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct Face {
    int v[3]; // 把每三个索引打包成一个三角形（面）
};

struct Edge {
    int v1, v2;
    int newVertex; // 新生成顶点的索引
    Edge() : v1(-1), v2(-1), newVertex(-1) {} // 默认构造函数
    Edge(int v1, int v2) : v1(v1), v2(v2), newVertex(-1) {}
    bool operator==(const Edge& other) const {
        return (v1 == other.v1 && v2 == other.v2) || (v1 == other.v2 && v2 == other.v1);
    }
};

struct PointHash {
    size_t operator()(const Point& point) const {
        return std::hash<float>()(point.x) ^ std::hash<float>()(point.y) ^ std::hash<float>()(point.z);
    }
};
struct PairHash {
    size_t operator()(const std::pair<int, int>& edge) const {
        return std::hash<int>()(edge.first) ^ std::hash<int>()(edge.second);
    }
};

void loopSubdivision(std::vector<float>& vertices, std::vector<int>& indices, int times) {
    std::vector<Point> points;
    std::vector<Face> faces;

    // 将 vertices 转换为 points
    points.reserve(vertices.size() / 3);
    for (size_t i = 0; i < vertices.size(); i += 3) {
        points.push_back(Point{vertices[i], vertices[i + 1], vertices[i + 2]});
    }

    // 将 indices 转换为 faces
    faces.reserve(indices.size() / 3);
    for (size_t i = 0; i < indices.size(); i += 3) {
        faces.push_back(Face{indices[i], indices[i + 1], indices[i + 2]});
    }

    for (int t = 0; t < times; ++t) {
        // 顶点索引 v1, v2 映射到 edge
        std::unordered_map<std::pair<int, int>, Edge, PairHash> edgeMap;
        // 顶点索引 v1, v2 映射到（edge 组成的两个三角形内）与该 edge 相对的两个顶点的 position
        std::unordered_map<std::pair<int, int>, std::vector<Point>, PairHash> edgeAdjacencyMap;
        // 每个旧顶点的相邻旧顶点，用 unordered_set 避免重复
        std::vector<std::unordered_set<Point, PointHash>> vertexAdjacencyList(points.size());

        // 填充 edgeMap 和 dgeAdjacencyMap
        for (const Face& face : faces) { // 对每个三角形
            for (int j = 0; j < 3; ++j) { // 遍历 3 条边
                int v1 = face.v[j];
                int v2 = face.v[(j + 1) % 3];
                if (v1 > v2) std::swap(v1, v2);
                auto edgeKey = std::make_pair(v1, v2);
                edgeMap[edgeKey] = Edge(v1, v2);
                auto& adj = edgeAdjacencyMap[edgeKey];
                adj.push_back(points[face.v[(j + 2) % 3]]); // 每个 face 内，每个 edge 对面的那个点
                vertexAdjacencyList[v1].insert(points[v2]);
                vertexAdjacencyList[v2].insert(points[v1]);
            }
        }

        // 计算新顶点位置: 0.375 * (A + B) + 0.125 * (C + D)
        std::vector<Point> newPoints = points; // 构建一个 newPoints，其中前 points.size() 为旧顶点，后 edgeMap.size() 为新顶点
        newPoints.reserve(points.size() + edgeMap.size());
        for (auto& edgePair : edgeMap) {
            Edge& edge = edgePair.second;
            Point A = points[edge.v1];
            Point B = points[edge.v2];
            auto& adj = edgeAdjacencyMap[edgePair.first];
            Point C = adj[0];   // 与边相对的两个顶点 C, D
            Point D = adj[1];   // note: adj.size() == 2

            Point newPoint = {
                3.0f / 8 * (A.x + B.x) + 1.0f / 8 * (C.x + D.x),
                3.0f / 8 * (A.y + B.y) + 1.0f / 8 * (C.y + D.y),
                3.0f / 8 * (A.z + B.z) + 1.0f / 8 * (C.z + D.z)
            };
            edge.newVertex = newPoints.size(); // 给它分配一个索引
            newPoints.push_back(newPoint);
        }

        // 计算原顶点的新位置: (1 - n*u) * old_pos + u * neighbor_pos
        for (size_t i = 0; i < points.size(); ++i) {
            Point sum = {0, 0, 0}; // 累计和
            int n = 0;             // degree
            for (auto& neighbor : vertexAdjacencyList[i]) {
                sum.x += neighbor.x;
                sum.y += neighbor.y;
                sum.z += neighbor.z;
                ++n;
            }
            float beta = (n == 3) ? 3.0 / 16 : 3.0 / (8 * n);
            newPoints[i].x = (1 - n * beta) * points[i].x + beta * sum.x;
            newPoints[i].y = (1 - n * beta) * points[i].y + beta * sum.y;
            newPoints[i].z = (1 - n * beta) * points[i].z + beta * sum.z;
        }

        // 生成新的面
        std::vector<Face> newFaces;
        newFaces.reserve(faces.size() * 4);
        for (const Face& face : faces) {
            int v0 = face.v[0];
            int v1 = face.v[1];
            int v2 = face.v[2];
            int e0 = edgeMap[{std::min(v0, v1), std::max(v0, v1)}].newVertex; // edge 上的 vertex
            int e1 = edgeMap[{std::min(v1, v2), std::max(v1, v2)}].newVertex;
            int e2 = edgeMap[{std::min(v2, v0), std::max(v2, v0)}].newVertex;

            newFaces.push_back(Face{v0, e0, e2});
            newFaces.push_back(Face{v1, e1, e0});
            newFaces.push_back(Face{v2, e2, e1});
            newFaces.push_back(Face{e0, e1, e2});
        }

        // 更新points和faces
        points = std::move(newPoints);
        faces = std::move(newFaces);
    }

    // 更新vertices和indices
    vertices.clear();
    indices.clear();
    vertices.reserve(points.size() * 3);
    indices.reserve(faces.size() * 3);
    for (const Point& p : points) {
        vertices.insert(vertices.end(), {p.x, p.y, p.z});
    }
    for (const Face& face : faces) {
        indices.insert(indices.end(), {face.v[0], face.v[1], face.v[2]});
    }
}