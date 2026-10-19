trait Container { type E; fn make() -> Self::E; }
struct Ring;
impl Container for Ring { type E=u32; fn make() -> u32 { 7 } }
fn main(){assert_eq!(Ring::make(),7);}
