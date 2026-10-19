use std::cell::Cell;
use std::num::NonZeroU32;
use std::mem::{size_of,align_of};
struct Resource<'a>{id:u32,drops:&'a Cell<u32>}
impl Drop for Resource<'_>{fn drop(&mut self){self.drops.set(self.drops.get()+1)}}
fn main(){
 let drops=Cell::new(0);let mut value=Some(Resource{id:0,drops:&drops});
 #[cfg(eop_bad_copy)] {let moved=value;let _again=value;drop(moved);return;}
 let moved=value.take();assert!(value.is_none());assert_eq!(moved.as_ref().unwrap().id,0);drop(moved);assert_eq!(drops.get(),1);
 assert_eq!(size_of::<Option<NonZeroU32>>(),size_of::<NonZeroU32>());assert_eq!(align_of::<Option<NonZeroU32>>(),align_of::<NonZeroU32>());
 assert_eq!(size_of::<Option<&u32>>(),size_of::<&u32>());assert_eq!(align_of::<Option<&u32>>(),align_of::<&u32>());
 assert_eq!(size_of::<Option<Box<u32>>>(),size_of::<Box<u32>>());assert_eq!(align_of::<Option<Box<u32>>>(),align_of::<Box<u32>>());
 println!("EOP_RUST zero-ID move-drop-once listed-niches u32-option-size{} u32-size{}",size_of::<Option<u32>>(),size_of::<u32>());
}
