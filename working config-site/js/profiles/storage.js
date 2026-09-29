const KEY='cobalt_profiles';
export function loadProfiles(){
  try{
    const raw=localStorage.getItem(KEY);
    return raw?JSON.parse(raw):[];
  }catch{return []}
}
export function saveProfiles(arr){
  localStorage.setItem(KEY,JSON.stringify(arr));
}
