#[derive(Copy, Clone)]
struct Vec3 {
	x: f32,
	y: f32,
	z: f32,
}

impl Vec3 {
	fn scale(self, s: f32) -> Self {
		Self {
			x: self.x * s,
			y: self.y * s,
			z: self.z * s,
		}
	}
}

fn main() -> () {
	let x = 1.0;
	let y = 2.0;
	
	let mut v = Vec3 { x, y, z: 3.0 };
}


