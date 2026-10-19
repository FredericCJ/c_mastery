use std::marker::PhantomData;
struct A;
#[allow(dead_code)] struct B;
struct Handle<T>{index:usize,marker:PhantomData<T>}
fn category(h:Handle<A>)->usize{h.index}
fn main(){assert_eq!(std::mem::size_of::<PhantomData<A>>(),0);
 #[cfg(wrong)] {let h=Handle::<B>{index:2,marker:PhantomData};assert_eq!(category(h),2);}
 #[cfg(not(wrong))] {let h=Handle::<A>{index:2,marker:PhantomData};let second=Handle::<A>{index:h.index,marker:PhantomData};assert_eq!(category(h),2);assert_eq!(category(second),2);}
}
