use fastanvil::{Chunk, ChunkData, CurrentJavaChunk, Section};
use fastanvil::pre13::{JavaChunk, Pre13Section};
use fastnbt::Value;
use crate::tile_entity::TileEntity;
use crate::world_fetcher::WorldFetcher;

pub struct TileEntityIterator<'a>{
    world_fetcher: &'a WorldFetcher<'a>
}

impl Iterator for TileEntityIterator{
    type Item = TileEntity;

    fn next(&mut self) -> Option<Self::Item> {

    }
}