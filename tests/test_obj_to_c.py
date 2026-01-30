import importlib.util
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "tools" / "obj_to_c.py"
spec = importlib.util.spec_from_file_location("obj_to_c", MODULE_PATH)
obj_to_c = importlib.util.module_from_spec(spec)
spec.loader.exec_module(obj_to_c)


class GenerateCTest(unittest.TestCase):
    def test_generate_c_writes_icosahedron_header_in_tests_folder(self):
        obj_path = ROOT / "tests" / "icosahedron.obj"
        output_path = ROOT / "tests" / "icosahedron_generated.h"

        vertices, faces, normals = obj_to_c.parse_and_process(obj_path)
        obj_to_c.generate_c(vertices, faces, normals, "icosahedron", output_path)

        content = output_path.read_text()
        self.assertTrue(output_path.exists())
        self.assertIn("#ifndef ICOSAHEDRON_H", content)
        self.assertIn("#define ICOSAHEDRON_H", content)
        self.assertIn('#include "common.h"', content)
        self.assertIn(
            f"#define _NUM_VERTS {len(vertices)}",
            content,
        )
        self.assertIn(
            f"#define _NUM_INDICES {len(faces)}",
            content,
        )
        self.assertIn("Triangle _indices[] = {", content)
        self.assertIn("short int _colorarray[] = {", content)
        self.assertTrue(content.rstrip().endswith("#endif"))

    def test_generate_c_writes_cube_header_in_tests_folder(self):
        obj_path = ROOT / "assets" / "models" / "cube.obj"
        output_path = ROOT / "tests" / "cube_generated.h"

        vertices, faces, normals = obj_to_c.parse_and_process(obj_path)
        obj_to_c.generate_c(vertices, faces, normals, "cube", output_path)

        content = output_path.read_text()
        self.assertTrue(output_path.exists())
        self.assertIn("#ifndef CUBE_H", content)
        self.assertIn("#define CUBE_H", content)
        self.assertIn('#include "common.h"', content)
        self.assertIn(
            f"#define _NUM_VERTS {len(vertices)}",
            content,
        )
        self.assertIn(
            f"#define _NUM_INDICES {len(faces)}",
            content,
        )
        self.assertIn("Triangle _indices[] = {", content)
        self.assertIn("short int _colorarray[] = {", content)
        self.assertTrue(content.rstrip().endswith("#endif"))


if __name__ == "__main__":
    unittest.main()
