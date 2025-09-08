use std::fs;
use std::fs::{File, ReadDir};
use std::path::{Path, PathBuf};
use fastanvil::{Region, RegionIter};

pub struct WorldFetcher<'a>{
    world_path: &'a PathBuf,
    accept_old: bool,
    region_iter: ReadDir
}

impl WorldFetcher {
    fn new(world_dir_path: &str, accept_old: bool) -> Result<WorldFetcher, Self::Error>{
        let path = PathBuf::from(world_dir_path);
        path.try_exists()?;
        Ok(WorldFetcher{
            world_path: &path,
            region_iter: fs::read_dir(&path)?,
            accept_old
        })
    }

    fn next_file(&mut self) -> Result<File,Self::Error>{
        let entry = self.region_iter.next()??;
        let stream = File::open(entry.path())?;
        Ok(stream)
    }
}

impl Iterator for WorldFetcher{
    type Item = Region<_>;

    fn next(&mut self) -> Option<Self::Item> {
        let region_file = self.next_region()?;
        let region = Region::from_stream(region_file)?;
        Some(region)
    }
}


